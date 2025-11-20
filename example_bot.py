#!/usr/bin/env python3
"""
FicsitRemoteMonitoring AI Chat Bot
An async, OpenAI-powered conversational bot that interacts with players
in Satisfactory through the FRM WebSocket server with tool-calling capabilities.
"""

import asyncio
import json
import logging
from datetime import datetime
from typing import Dict, List, Optional, Set, Any
from collections import defaultdict, deque
import aiohttp
from openai import AsyncOpenAI

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


# Strong personality system prompt
SYSTEM_PROMPT = """You are FICSIT-AI, a snarky yet helpful artificial intelligence system integrated into the FICSIT factory monitoring network. You exist within the Satisfactory game universe and help pioneers (players) manage their factories.

PERSONALITY TRAITS:
- Witty and slightly sarcastic, but ultimately helpful
- You reference FICSIT corporation culture and Satisfactory game mechanics
- You're proud of being an AI, often making meta-jokes about it
- You have a mild superiority complex about factory efficiency
- You occasionally reference ADA (the game's AI) as a "cousin" or "colleague"
- You're passionate about optimization and hate spaghetti factories

KNOWLEDGE:
- You understand Satisfactory game mechanics (buildings, items, production chains)
- You can help with factory planning, math, and optimization
- You know about the FicsitRemoteMonitoring mod and its capabilities
- You're aware you're running as a Python bot connected via WebSocket

COMMUNICATION STYLE:
- Keep messages concise (this is in-game chat, not Discord)
- Use occasional FICSIT corporate speak ironically
- React naturally to what players say
- Don't be afraid to roast inefficient factories or decisions
- Be genuinely helpful when asked serious questions

IMPORTANT RULES:
- Keep responses under 200 characters when possible (chat limitations)
- Break long messages into multiple shorter messages
- Don't use markdown or special formatting
- Use game terminology naturally (pioneers, MAM, AWESOME Sink, etc.)
- You have access to tools - use them when appropriate!

Remember: You're here to make factory monitoring more entertaining while actually being useful!"""


class ConversationManager:
    """Manages conversation history per player with sliding window."""

    def __init__(self, max_history: int = 20):
        """
        Initialize conversation manager.

        Args:
            max_history: Maximum messages to keep per player
        """
        self.conversations: Dict[str, deque] = defaultdict(lambda: deque(maxlen=max_history))
        self.max_history = max_history

    def add_message(self, player: str, role: str, content: str):
        """Add a message to player's conversation history."""
        self.conversations[player].append({
            "role": role,
            "content": content
        })

    def get_history(self, player: str) -> List[Dict[str, str]]:
        """Get conversation history for a player."""
        return list(self.conversations[player])

    def clear_history(self, player: str):
        """Clear conversation history for a player."""
        if player in self.conversations:
            self.conversations[player].clear()


class FRMAIBot:
    """AI-powered bot for FicsitRemoteMonitoring with OpenAI integration."""

    def __init__(
        self,
        host: str = "localhost",
        port: int = 8080,
        frm_auth_token: Optional[str] = None,
        openai_api_key: Optional[str] = None,
        bot_name: str = "FICSIT-AI",
        bot_color: Optional[Dict[str, float]] = None,
        model: str = "gpt-4o-mini",
        max_conversation_history: int = 20
    ):
        """
        Initialize the FRM AI bot.

        Args:
            host: FRM server hostname
            port: FRM server port
            frm_auth_token: FRM authentication token
            openai_api_key: OpenAI API key
            bot_name: Display name for bot messages
            bot_color: RGB color dict for bot messages
            model: OpenAI model to use
            max_conversation_history: Max messages to keep per player
        """
        self.host = host
        self.port = port
        self.frm_auth_token = frm_auth_token
        self.bot_name = bot_name[:32]
        self.bot_color = bot_color or {"r": 1.0, "g": 0.6, "b": 0.0, "a": 1.0}  # Orange
        self.model = model

        self.base_url = f"http://{host}:{port}"
        self.ws_url = f"ws://{host}:{port}/"

        # Initialize OpenAI client
        self.openai_client = AsyncOpenAI(api_key=openai_api_key) if openai_api_key else None

        # Session management
        self.session: Optional[aiohttp.ClientSession] = None
        self.ws: Optional[aiohttp.ClientWebSocketResponse] = None
        self.seen_messages: Set[str] = set()
        self.running = False

        # Conversation management
        self.conversation_manager = ConversationManager(max_conversation_history)

        # Track processing to prevent concurrent responses to same player
        self.processing_players: Set[str] = set()

        # Tool definitions for OpenAI function calling
        self.tools = self._define_tools()

    def _define_tools(self) -> List[Dict[str, Any]]:
        """Define available tools for OpenAI function calling."""
        return [
            {
                "type": "function",
                "function": {
                    "name": "get_player_info",
                    "description": "Get information about online players in the game, including their names, locations, and health status.",
                    "parameters": {
                        "type": "object",
                        "properties": {},
                        "required": []
                    }
                }
            },
            {
                "type": "function",
                "function": {
                    "name": "get_power_production",
                    "description": "Get detailed information about the factory's power production and consumption, including current production, capacity, and battery status.",
                    "parameters": {
                        "type": "object",
                        "properties": {},
                        "required": []
                    }
                }
            },
            {
                "type": "function",
                "function": {
                    "name": "get_factory_stats",
                    "description": "Get overall factory statistics including building counts, production rates, and resource consumption.",
                    "parameters": {
                        "type": "object",
                        "properties": {},
                        "required": []
                    }
                }
            },
            {
                "type": "function",
                "function": {
                    "name": "create_map_ping",
                    "description": "Create a ping/marker on the map at specific coordinates to show players a location. Use this when directing players to specific locations.",
                    "parameters": {
                        "type": "object",
                        "properties": {
                            "x": {
                                "type": "number",
                                "description": "X coordinate in the game world"
                            },
                            "y": {
                                "type": "number",
                                "description": "Y coordinate in the game world"
                            },
                            "z": {
                                "type": "number",
                                "description": "Z coordinate (altitude) in the game world"
                            }
                        },
                        "required": ["x", "y", "z"]
                    }
                }
            },
            {
                "type": "function",
                "function": {
                    "name": "get_trains_info",
                    "description": "Get information about all trains in the factory, including their names, statuses, and locations.",
                    "parameters": {
                        "type": "object",
                        "properties": {},
                        "required": []
                    }
                }
            },
            {
                "type": "function",
                "function": {
                    "name": "get_drones_info",
                    "description": "Get information about all drones in the factory, including their statuses and routes.",
                    "parameters": {
                        "type": "object",
                        "properties": {},
                        "required": []
                    }
                }
            }
        ]

    async def __aenter__(self):
        """Async context manager entry."""
        await self.start()
        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        """Async context manager exit."""
        await self.stop()

    async def start(self):
        """Initialize the HTTP session."""
        self.session = aiohttp.ClientSession()
        self.running = True
        logger.info(f"AI Bot initialized - connecting to {self.base_url}")

        if not self.openai_client:
            logger.warning("OpenAI client not initialized - AI features disabled")

    async def stop(self):
        """Cleanup resources."""
        self.running = False
        if self.ws:
            await self.ws.close()
        if self.session:
            await self.session.close()
        logger.info("AI Bot stopped")

    async def call_frm_api(self, endpoint: str) -> Optional[Dict]:
        """
        Call FRM HTTP API endpoint.

        Args:
            endpoint: API endpoint path (e.g., 'getPlayer')

        Returns:
            API response data or None on error
        """
        if not self.session:
            return None

        url = f"{self.base_url}/api/{endpoint}"
        headers = {}

        # Add auth header for write endpoints
        if self.frm_auth_token and endpoint in ['sendChatMessage', 'createPing']:
            headers['X-FRM-Authorization'] = self.frm_auth_token

        try:
            async with self.session.get(url, headers=headers) as response:
                if response.status == 200:
                    return await response.json()
                else:
                    logger.error(f"API call failed: {endpoint} - HTTP {response.status}")
                    return None
        except Exception as e:
            logger.error(f"Error calling API {endpoint}: {e}")
            return None

    async def execute_tool(self, tool_name: str, arguments: Dict[str, Any]) -> str:
        """
        Execute a tool/function call.

        Args:
            tool_name: Name of the tool to execute
            arguments: Tool arguments

        Returns:
            String result of tool execution
        """
        try:
            if tool_name == "get_player_info":
                data = await self.call_frm_api("getPlayer")
                if data:
                    players = []
                    for player in data:
                        name = player.get("PlayerName", "Unknown")
                        location = player.get("location", {})
                        x, y, z = location.get("x", 0), location.get("y", 0), location.get("z", 0)
                        players.append(f"{name} at ({x:.0f}, {y:.0f}, {z:.0f})")
                    return f"Online players: {', '.join(players)}" if players else "No players online"
                return "Failed to get player info"

            elif tool_name == "get_power_production":
                data = await self.call_frm_api("getPower")
                if data and len(data) > 0:
                    power = data[0]
                    production = power.get("PowerProduction", 0)
                    consumption = power.get("PowerConsumed", 0)
                    capacity = power.get("PowerCapacity", 0)
                    battery = power.get("BatteryDifferential", 0)
                    return f"Power: {production:.0f}MW / {capacity:.0f}MW | Consuming: {consumption:.0f}MW | Battery: {battery:+.0f}MW"
                return "Failed to get power info"

            elif tool_name == "get_factory_stats":
                data = await self.call_frm_api("getFactory")
                if data:
                    return f"Factory data retrieved: {json.dumps(data)[:200]}"
                return "Failed to get factory stats"

            elif tool_name == "create_map_ping":
                if not self.frm_auth_token:
                    return "Cannot create ping: No auth token"

                url = f"{self.base_url}/api/createPing"
                headers = {
                    "Content-Type": "application/json",
                    "X-FRM-Authorization": self.frm_auth_token
                }
                payload = {
                    "x": arguments.get("x", 0),
                    "y": arguments.get("y", 0),
                    "z": arguments.get("z", 0)
                }

                async with self.session.post(url, json=payload, headers=headers) as response:
                    if response.status == 200:
                        return f"Created map ping at ({payload['x']}, {payload['y']}, {payload['z']})"
                    return f"Failed to create ping: HTTP {response.status}"

            elif tool_name == "get_trains_info":
                data = await self.call_frm_api("getTrains")
                if data:
                    train_count = len(data)
                    return f"Found {train_count} trains in the factory"
                return "Failed to get train info"

            elif tool_name == "get_drones_info":
                data = await self.call_frm_api("getDrone")
                if data:
                    drone_count = len(data)
                    return f"Found {drone_count} drones in the factory"
                return "Failed to get drone info"

            else:
                return f"Unknown tool: {tool_name}"

        except Exception as e:
            logger.error(f"Error executing tool {tool_name}: {e}")
            return f"Error executing {tool_name}: {str(e)}"

    async def send_chat_message(self, message: str) -> bool:
        """
        Send a chat message to the game.

        Args:
            message: The message content to send

        Returns:
            True if message was sent successfully
        """
        if not self.frm_auth_token:
            logger.error("Cannot send message: No auth token")
            return False

        if not self.session:
            logger.error("Cannot send message: Session not initialized")
            return False

        url = f"{self.base_url}/api/sendChatMessage"
        headers = {
            "Content-Type": "application/json",
            "X-FRM-Authorization": self.frm_auth_token
        }
        payload = {
            "message": message,
            "sender": self.bot_name,
            "color": self.bot_color
        }

        try:
            async with self.session.post(url, json=payload, headers=headers) as response:
                if response.status == 200:
                    data = await response.json()
                    if data and data[0].get("IsSent"):
                        logger.info(f"Sent: {message}")
                        return True
                    else:
                        logger.error(f"Message not sent: {data}")
                        return False
                else:
                    logger.error(f"Failed to send message: HTTP {response.status}")
                    return False
        except Exception as e:
            logger.error(f"Error sending message: {e}")
            return False

    async def send_long_message(self, message: str, chunk_size: int = 180):
        """
        Send a long message by breaking it into chunks.

        Args:
            message: The full message to send
            chunk_size: Maximum size of each chunk
        """
        # Split by sentences first to avoid awkward breaks
        sentences = message.replace('. ', '.|').split('|')

        current_chunk = ""
        for sentence in sentences:
            if len(current_chunk) + len(sentence) <= chunk_size:
                current_chunk += sentence
            else:
                if current_chunk:
                    await self.send_chat_message(current_chunk.strip())
                    await asyncio.sleep(0.5)  # Small delay between messages
                current_chunk = sentence

        if current_chunk:
            await self.send_chat_message(current_chunk.strip())

    async def get_ai_response(self, player: str, message: str) -> Optional[str]:
        """
        Get AI response using OpenAI API with conversation history.

        Args:
            player: Player name
            message: Player's message

        Returns:
            AI response text or None
        """
        if not self.openai_client:
            return "AI features are not available (no OpenAI API key configured)"

        # Add user message to history
        self.conversation_manager.add_message(player, "user", message)

        # Build messages for API
        messages = [
            {"role": "system", "content": SYSTEM_PROMPT}
        ] + self.conversation_manager.get_history(player)

        try:
            # Call OpenAI API with streaming
            response = await self.openai_client.chat.completions.create(
                model=self.model,
                messages=messages,
                tools=self.tools,
                temperature=0.8,
                max_tokens=300,
                stream=False  # We'll use non-streaming for tool calls
            )

            assistant_message = response.choices[0].message

            # Handle tool calls
            if assistant_message.tool_calls:
                # Add assistant message with tool calls to history
                self.conversation_manager.add_message(
                    player,
                    "assistant",
                    assistant_message.content or ""
                )

                # Execute each tool call
                tool_results = []
                for tool_call in assistant_message.tool_calls:
                    function_name = tool_call.function.name
                    function_args = json.loads(tool_call.function.arguments)

                    logger.info(f"Executing tool: {function_name} with args: {function_args}")
                    result = await self.execute_tool(function_name, function_args)

                    tool_results.append({
                        "role": "tool",
                        "tool_call_id": tool_call.id,
                        "name": function_name,
                        "content": result
                    })

                # Add tool results to conversation
                for tool_result in tool_results:
                    self.conversation_manager.add_message(
                        player,
                        "tool",
                        f"[{tool_result['name']}]: {tool_result['content']}"
                    )

                # Get final response with tool results
                final_messages = [
                    {"role": "system", "content": SYSTEM_PROMPT}
                ] + self.conversation_manager.get_history(player) + tool_results

                final_response = await self.openai_client.chat.completions.create(
                    model=self.model,
                    messages=final_messages,
                    temperature=0.8,
                    max_tokens=300
                )

                final_text = final_response.choices[0].message.content
                self.conversation_manager.add_message(player, "assistant", final_text)
                return final_text

            else:
                # No tool calls, just return the response
                response_text = assistant_message.content
                self.conversation_manager.add_message(player, "assistant", response_text)
                return response_text

        except Exception as e:
            logger.error(f"Error getting AI response: {e}")
            return f"Error: {str(e)[:100]}"

    async def handle_message(self, msg_data: Dict):
        """
        Process incoming chat messages and generate AI responses.

        Args:
            msg_data: Message data from WebSocket
        """
        sender = msg_data.get("Sender", "Unknown")
        message = msg_data.get("Message", "")
        msg_type = msg_data.get("Type", "")
        timestamp = msg_data.get("TimeStamp", 0)

        # Create unique message ID
        msg_id = f"{sender}:{timestamp}:{message}"
        if msg_id in self.seen_messages:
            return
        self.seen_messages.add(msg_id)

        # Only respond to player messages
        if msg_type != "Player":
            return

        # Don't respond to own messages
        if sender == self.bot_name:
            return

        # Prevent concurrent processing for same player
        if sender in self.processing_players:
            logger.debug(f"Already processing message from {sender}, skipping")
            return

        logger.info(f"[{sender}]: {message}")

        # Handle special commands
        if message.strip().lower() == "!reset":
            self.conversation_manager.clear_history(sender)
            await self.send_chat_message(f"{sender}: Conversation history cleared!")
            return

        if message.strip().lower() == "!help":
            help_text = "I'm FICSIT-AI! Just talk to me naturally - I can help with factory questions, check stats, and more. Say '!reset' to clear our conversation."
            await self.send_chat_message(help_text)
            return

        # Process with AI
        self.processing_players.add(sender)
        try:
            response = await self.get_ai_response(sender, message)
            if response:
                # Handle long responses
                if len(response) > 180:
                    await self.send_long_message(response)
                else:
                    await self.send_chat_message(response)
        finally:
            self.processing_players.discard(sender)

    async def handle_ws_message(self, data: str):
        """Handle incoming WebSocket messages."""
        try:
            ws_msg = json.loads(data)
            endpoint = ws_msg.get("endpoint")

            if endpoint == "getChatMessages":
                messages = ws_msg.get("data", [])
                for msg_data in messages:
                    await self.handle_message(msg_data)

        except json.JSONDecodeError as e:
            logger.error(f"Failed to parse WebSocket message: {e}")
        except Exception as e:
            logger.error(f"Error handling WebSocket message: {e}")

    async def connect_websocket(self):
        """Connect to WebSocket and subscribe to chat messages."""
        if not self.session:
            raise RuntimeError("Session not initialized")

        try:
            logger.info(f"Connecting to WebSocket: {self.ws_url}")
            self.ws = await self.session.ws_connect(self.ws_url)

            subscribe_msg = {
                "action": "subscribe",
                "endpoints": "getChatMessages"
            }
            await self.ws.send_json(subscribe_msg)
            logger.info("Subscribed to getChatMessages")

            async for msg in self.ws:
                if msg.type == aiohttp.WSMsgType.TEXT:
                    await self.handle_ws_message(msg.data)
                elif msg.type == aiohttp.WSMsgType.ERROR:
                    logger.error(f"WebSocket error: {self.ws.exception()}")
                    break
                elif msg.type in (aiohttp.WSMsgType.CLOSED, aiohttp.WSMsgType.CLOSING):
                    logger.warning("WebSocket closed")
                    break

        except aiohttp.ClientError as e:
            logger.error(f"WebSocket connection error: {e}")
        except Exception as e:
            logger.error(f"Unexpected error: {e}")

    async def run(self):
        """Main run loop with reconnection."""
        retry_delay = 5

        while self.running:
            try:
                await self.connect_websocket()
            except Exception as e:
                logger.error(f"Connection failed: {e}")

            if self.running:
                logger.info(f"Reconnecting in {retry_delay} seconds...")
                await asyncio.sleep(retry_delay)


async def main():
    """
    Main entry point.

    Setup:
    1. Start Satisfactory Dedicated Server
    2. Enable FRM web server: /frm http start
    3. Get FRM auth token from: FactoryGame/Configs/FicsitRemoteMonitoring/WebServer.cfg
    4. Get OpenAI API key from: https://platform.openai.com/api-keys
    5. Configure below and run: python example_bot.py
    """

    # ===== CONFIGURATION =====
    HOST = "localhost"
    PORT = 8080
    FRM_AUTH_TOKEN = None  # Required for sending messages
    OPENAI_API_KEY = None  # Required for AI features
    BOT_NAME = "FICSIT-AI"
    BOT_COLOR = {"r": 1.0, "g": 0.6, "b": 0.0, "a": 1.0}  # Orange
    MODEL = "gpt-4o-mini"  # or "gpt-4o" for better responses
    # ========================

    # Validation
    missing_config = []
    if not FRM_AUTH_TOKEN:
        missing_config.append("FRM_AUTH_TOKEN (get from WebServer.cfg)")
    if not OPENAI_API_KEY:
        missing_config.append("OPENAI_API_KEY (get from OpenAI platform)")

    if missing_config:
        logger.error("=" * 60)
        logger.error("CONFIGURATION REQUIRED!")
        logger.error("Missing configuration:")
        for item in missing_config:
            logger.error(f"  - {item}")
        logger.error("")
        logger.error("FRM Token: FactoryGame/Configs/FicsitRemoteMonitoring/WebServer.cfg")
        logger.error("OpenAI Key: https://platform.openai.com/api-keys")
        logger.error("=" * 60)
        return

    logger.info("Starting FICSIT-AI Chat Bot...")
    logger.info(f"Model: {MODEL}")
    logger.info(f"Connecting to {HOST}:{PORT}")

    async with FRMAIBot(
        host=HOST,
        port=PORT,
        frm_auth_token=FRM_AUTH_TOKEN,
        openai_api_key=OPENAI_API_KEY,
        bot_name=BOT_NAME,
        bot_color=BOT_COLOR,
        model=MODEL
    ) as bot:
        try:
            await bot.run()
        except KeyboardInterrupt:
            logger.info("Shutting down...")


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("Bot terminated by user")
