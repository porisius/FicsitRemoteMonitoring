#!/usr/bin/env python3
"""
FicsitRemoteMonitoring Chat Bot Example
An async Python script that connects to the FRM WebSocket server,
receives player messages, and responds based on content.
"""

import asyncio
import json
import logging
from datetime import datetime
from typing import Dict, Optional, Set
import aiohttp

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class FRMBot:
    """Async bot for interacting with FicsitRemoteMonitoring chat."""

    def __init__(
        self,
        host: str = "localhost",
        port: int = 8080,
        auth_token: Optional[str] = None,
        bot_name: str = "FRM Bot",
        bot_color: Optional[Dict[str, float]] = None
    ):
        """
        Initialize the FRM bot.

        Args:
            host: Server hostname (default: localhost)
            port: Server port (default: 8080)
            auth_token: Authentication token from WebServer.cfg
            bot_name: Display name for bot messages (max 32 chars)
            bot_color: RGB color dict with keys 'r', 'g', 'b', 'a' (0-1 range)
        """
        self.host = host
        self.port = port
        self.auth_token = auth_token
        self.bot_name = bot_name[:32]  # Enforce max length
        self.bot_color = bot_color or {"r": 0.2, "g": 0.8, "b": 1.0, "a": 1.0}

        self.base_url = f"http://{host}:{port}"
        self.ws_url = f"ws://{host}:{port}/"

        self.session: Optional[aiohttp.ClientSession] = None
        self.ws: Optional[aiohttp.ClientWebSocketResponse] = None
        self.seen_messages: Set[str] = set()  # Prevent duplicate processing
        self.running = False

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
        logger.info(f"Bot initialized - connecting to {self.base_url}")

    async def stop(self):
        """Cleanup resources."""
        self.running = False
        if self.ws:
            await self.ws.close()
        if self.session:
            await self.session.close()
        logger.info("Bot stopped")

    async def send_chat_message(self, message: str) -> bool:
        """
        Send a chat message to the game.

        Args:
            message: The message content to send

        Returns:
            True if message was sent successfully, False otherwise
        """
        if not self.auth_token:
            logger.error("Cannot send message: No auth token configured")
            return False

        if not self.session:
            logger.error("Cannot send message: Session not initialized")
            return False

        url = f"{self.base_url}/api/sendChatMessage"
        headers = {
            "Content-Type": "application/json",
            "X-FRM-Authorization": self.auth_token
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
                        logger.info(f"Sent message: {message}")
                        return True
                    else:
                        logger.error(f"Message not sent: {data}")
                        return False
                elif response.status == 401:
                    logger.error("Authentication failed - check your token")
                    return False
                else:
                    logger.error(f"Failed to send message: HTTP {response.status}")
                    return False
        except Exception as e:
            logger.error(f"Error sending message: {e}")
            return False

    async def handle_message(self, msg_data: Dict):
        """
        Process incoming chat messages and respond based on content.

        Args:
            msg_data: Message data from WebSocket
        """
        sender = msg_data.get("Sender", "Unknown")
        message = msg_data.get("Message", "")
        msg_type = msg_data.get("Type", "")
        timestamp = msg_data.get("TimeStamp", 0)

        # Create unique message ID to prevent duplicate processing
        msg_id = f"{sender}:{timestamp}:{message}"
        if msg_id in self.seen_messages:
            return
        self.seen_messages.add(msg_id)

        # Only respond to player messages (not system/Ada messages)
        if msg_type != "Player":
            return

        # Don't respond to our own messages
        if sender == self.bot_name:
            return

        logger.info(f"[{sender}]: {message}")

        # Command parsing - messages starting with '!'
        if message.startswith("!"):
            await self.handle_command(sender, message)

    async def handle_command(self, sender: str, message: str):
        """
        Handle bot commands.

        Args:
            sender: The player who sent the command
            message: The command message
        """
        parts = message.split(maxsplit=1)
        command = parts[0].lower()
        args = parts[1] if len(parts) > 1 else ""

        if command == "!help":
            response = (
                f"Hello {sender}! Available commands: "
                "!help, !echo <text>, !time, !ping, !reverse <text>"
            )
            await self.send_chat_message(response)

        elif command == "!echo":
            if args:
                await self.send_chat_message(f"{sender} says: {args}")
            else:
                await self.send_chat_message("Usage: !echo <text>")

        elif command == "!time":
            current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            await self.send_chat_message(f"Current time: {current_time}")

        elif command == "!ping":
            await self.send_chat_message(f"Pong! Hello {sender}!")

        elif command == "!reverse":
            if args:
                reversed_text = args[::-1]
                await self.send_chat_message(f"{reversed_text}")
            else:
                await self.send_chat_message("Usage: !reverse <text>")

        else:
            # Unknown command - optionally respond or ignore
            logger.debug(f"Unknown command: {command}")

    async def connect_websocket(self):
        """
        Connect to the WebSocket server and subscribe to chat messages.
        """
        if not self.session:
            raise RuntimeError("Session not initialized - call start() first")

        try:
            logger.info(f"Connecting to WebSocket: {self.ws_url}")
            self.ws = await self.session.ws_connect(self.ws_url)

            # Subscribe to chat messages
            subscribe_msg = {
                "action": "subscribe",
                "endpoints": "getChatMessages"
            }
            await self.ws.send_json(subscribe_msg)
            logger.info("Subscribed to getChatMessages")

            # Listen for messages
            async for msg in self.ws:
                if msg.type == aiohttp.WSMsgType.TEXT:
                    await self.handle_ws_message(msg.data)
                elif msg.type == aiohttp.WSMsgType.ERROR:
                    logger.error(f"WebSocket error: {self.ws.exception()}")
                    break
                elif msg.type in (aiohttp.WSMsgType.CLOSED, aiohttp.WSMsgType.CLOSING):
                    logger.warning("WebSocket connection closed")
                    break

        except aiohttp.ClientError as e:
            logger.error(f"WebSocket connection error: {e}")
        except Exception as e:
            logger.error(f"Unexpected error in WebSocket: {e}")

    async def handle_ws_message(self, data: str):
        """
        Handle incoming WebSocket messages.

        Args:
            data: Raw WebSocket message data (JSON string)
        """
        try:
            ws_msg = json.loads(data)
            endpoint = ws_msg.get("endpoint")

            if endpoint == "getChatMessages":
                messages = ws_msg.get("data", [])
                # Process each message
                for msg_data in messages:
                    await self.handle_message(msg_data)

        except json.JSONDecodeError as e:
            logger.error(f"Failed to parse WebSocket message: {e}")
        except Exception as e:
            logger.error(f"Error handling WebSocket message: {e}")

    async def run(self):
        """
        Main run loop - connects to WebSocket and handles reconnection.
        """
        retry_delay = 5  # seconds

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
    Main entry point for the bot.

    To use this bot:
    1. Start your Satisfactory Dedicated Server
    2. Enable the FRM web server: /frm http start
    3. Get your auth token from: FactoryGame/Configs/FicsitRemoteMonitoring/WebServer.cfg
    4. Set the AUTH_TOKEN variable below
    5. Run this script: python example_bot.py
    """

    # ===== CONFIGURATION =====
    HOST = "localhost"
    PORT = 8080
    AUTH_TOKEN = None  # Set this to your token from WebServer.cfg
    BOT_NAME = "FRM Bot"
    BOT_COLOR = {"r": 0.2, "g": 0.8, "b": 1.0, "a": 1.0}  # Nice cyan color
    # ========================

    if not AUTH_TOKEN:
        logger.warning("=" * 60)
        logger.warning("WARNING: No auth token set!")
        logger.warning("The bot will receive messages but cannot send replies.")
        logger.warning("To enable sending:")
        logger.warning("1. Find your token in: FactoryGame/Configs/")
        logger.warning("   FicsitRemoteMonitoring/WebServer.cfg")
        logger.warning("2. Set AUTH_TOKEN in this script")
        logger.warning("=" * 60)

    logger.info("Starting FicsitRemoteMonitoring Chat Bot...")
    logger.info(f"Connecting to {HOST}:{PORT}")

    async with FRMBot(
        host=HOST,
        port=PORT,
        auth_token=AUTH_TOKEN,
        bot_name=BOT_NAME,
        bot_color=BOT_COLOR
    ) as bot:
        try:
            await bot.run()
        except KeyboardInterrupt:
            logger.info("Shutting down bot...")


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        logger.info("Bot terminated by user")
