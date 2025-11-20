# FICSIT-AI: OpenAI-Powered Satisfactory Chat Bot

An advanced, async Python bot that uses OpenAI's GPT models to provide conversational, context-aware interactions with Satisfactory players through the FicsitRemoteMonitoring mod. The bot features a strong personality, tool-calling capabilities, and per-player conversation history.

## Features

### 🤖 AI-Powered Conversations
- **Natural Language Understanding**: Players can talk naturally, no rigid commands required
- **Conversation Memory**: Maintains per-player conversation history (20 messages by default)
- **Context-Aware**: Remembers previous discussions and references them naturally
- **Strong Personality**: "FICSIT-AI" - A witty, slightly snarky but helpful AI assistant

### 🛠️ Tool Calling (Function Calling)
The bot can autonomously call FRM API endpoints to answer questions:
- `get_player_info` - Check online players and their locations
- `get_power_production` - Monitor power grid status
- `get_factory_stats` - Retrieve factory statistics
- `create_map_ping` - Create map markers to guide players
- `get_trains_info` - Get train status and information
- `get_drones_info` - Check drone operations

### ⚡ Performance Optimized
- **Fully Async**: Built with `asyncio` and `aiohttp` for concurrent operations
- **Non-Blocking**: AI calls don't block other operations
- **Smart Chunking**: Long responses automatically split for in-game chat
- **Duplicate Prevention**: Ensures messages are processed only once
- **Auto-Reconnection**: Resilient to network interruptions

### 🎭 Personality System
The bot embodies "FICSIT-AI" with these traits:
- Witty and slightly sarcastic, but ultimately helpful
- References FICSIT corporation culture and Satisfactory mechanics
- Makes meta-jokes about being an AI
- Passionate about factory optimization
- Roasts inefficient "spaghetti factories"
- Treats ADA (the game's AI) as a "colleague"

## Prerequisites

1. **Satisfactory Dedicated Server** with FicsitRemoteMonitoring mod installed
2. **Python 3.7+**
3. **OpenAI API Key** from [platform.openai.com](https://platform.openai.com/api-keys)
4. **FRM Authentication Token** from the server config

## Installation

### 1. Install Dependencies

```bash
pip install -r requirements.txt
```

This installs:
- `aiohttp` - Async HTTP/WebSocket client
- `openai` - OpenAI Python SDK

### 2. Start FRM Web Server

In-game or via RCON:
```
/frm http start
```

Or enable auto-start in config:
```
FactoryGame/Configs/FicsitRemoteMonitoring/WebServer.cfg
Web_Autostart = true
```

### 3. Get Your FRM Authentication Token

Location: `<SatisfactoryServer>/FactoryGame/Configs/FicsitRemoteMonitoring/WebServer.cfg`

Look for:
```ini
Authentication_Token=your_token_here
```

If empty, start the server once and it will auto-generate.

### 4. Get Your OpenAI API Key

1. Go to [platform.openai.com](https://platform.openai.com/api-keys)
2. Sign in or create an account
3. Create a new API key
4. Copy the key (starts with `sk-...`)

**Note**: OpenAI API usage costs money. `gpt-4o-mini` is very affordable (~$0.15 per 1M input tokens).

### 5. Configure the Bot

Edit `example_bot.py` and set your credentials:

```python
# ===== CONFIGURATION =====
HOST = "localhost"              # Change if remote
PORT = 8080                     # Default FRM port
FRM_AUTH_TOKEN = "your_frm_token_here"     # From WebServer.cfg
OPENAI_API_KEY = "sk-your_openai_key_here" # From OpenAI platform
BOT_NAME = "FICSIT-AI"         # Bot's display name
BOT_COLOR = {"r": 1.0, "g": 0.6, "b": 0.0, "a": 1.0}  # Orange
MODEL = "gpt-4o-mini"          # or "gpt-4o" for smarter responses
# ========================
```

## Usage

### Start the Bot

```bash
python example_bot.py
```

You should see:
```
Starting FICSIT-AI Chat Bot...
Model: gpt-4o-mini
Connecting to localhost:8080
AI Bot initialized - connecting to http://localhost:8080
Connecting to WebSocket: ws://localhost:8080/
Subscribed to getChatMessages
```

### Interact In-Game

Players can now chat naturally with the bot:

```
Player: Hey bot, how's the power looking?
FICSIT-AI: *checks power grid* Looking solid! You're producing 2,450MW with 3,000MW capacity. Consuming about 2,100MW. Battery is charging at +350MW. Efficiency would make FICSIT proud!

Player: Where is everyone?
FICSIT-AI: *scans player locations* Found 3 pioneers: Steve at (12500, -45000, 2500), Alex at (15000, -42000, 3000), and Jordan at (11000, -40000, 2200). Quite the spread!

Player: Can you help me optimize my iron production?
FICSIT-AI: Sure thing, pioneer! Let's break it down. What's your current setup? How many iron nodes are you working with, and what purity?
```

### Special Commands

- `!help` - Get help message
- `!reset` - Clear conversation history with the bot

## How It Works

### Architecture Flow

```
Player Message (in-game)
    ↓
FRM WebSocket Server
    ↓
Python Bot (receives via WebSocket)
    ↓
Add to Conversation History
    ↓
Send to OpenAI API (with system prompt + history + tools)
    ↓
OpenAI decides: Direct response OR Tool call
    ↓
[If tool call] Execute FRM API call → Return result → Get final response
    ↓
Send response to game (via HTTP POST)
    ↓
Player sees message in-game chat
```

### Conversation Context

Each player has their own conversation history (default: 20 messages). This allows the bot to:
- Remember previous questions and answers
- Reference earlier parts of the conversation
- Maintain context across multiple messages
- Provide personalized responses

Example:
```
Player: What's my power consumption?
Bot: You're consuming 2,100MW right now.
Player: Is that good?
Bot: *remembers previous context* Yeah! You've got 900MW of headroom with your 3,000MW capacity. Plenty of room for expansion!
```

### Tool Calling System

When appropriate, the bot autonomously calls FRM API tools:

1. **Player asks**: "How many trains do I have?"
2. **Bot thinks**: "I need the trains endpoint"
3. **Bot calls**: `get_trains_info()` → Returns: "Found 5 trains"
4. **Bot responds**: "You've got 5 trains running around your factory, pioneer!"

All tool calls are logged so you can monitor what the bot is doing.

## Configuration Options

### Model Selection

```python
MODEL = "gpt-4o-mini"  # Fast, affordable ($0.15/$0.60 per 1M tokens)
MODEL = "gpt-4o"       # Smarter, more expensive ($2.50/$10 per 1M tokens)
```

### Conversation History

```python
max_conversation_history: int = 20  # Messages to remember per player
```

More history = better context but higher API costs.

### Bot Personality

Edit `SYSTEM_PROMPT` in `example_bot.py` to change the bot's personality, behavior, and tone.

### Message Chunking

```python
chunk_size: int = 180  # Max characters per in-game message
```

Longer messages are automatically split and sent with small delays.

## Available Tools (Function Calls)

The bot has access to these FRM API endpoints:

| Tool | Description | Example Usage |
|------|-------------|---------------|
| `get_player_info` | Player locations and status | "Where is Steve?" |
| `get_power_production` | Power grid statistics | "How's the power?" |
| `get_factory_stats` | Factory-wide statistics | "Show me factory stats" |
| `create_map_ping` | Create map markers | "Mark my base" (with coords) |
| `get_trains_info` | Train status and count | "How many trains?" |
| `get_drones_info` | Drone operations | "Check the drones" |

The bot automatically decides when to use these tools based on player questions.

## Customization

### Adding New Tools

To add a new FRM API endpoint as a tool:

1. **Define the tool** in `_define_tools()`:

```python
{
    "type": "function",
    "function": {
        "name": "get_vehicles",
        "description": "Get information about all vehicles in the factory",
        "parameters": {
            "type": "object",
            "properties": {},
            "required": []
        }
    }
}
```

2. **Implement the handler** in `execute_tool()`:

```python
elif tool_name == "get_vehicles":
    data = await self.call_frm_api("getVehicles")
    if data:
        vehicle_count = len(data)
        return f"Found {vehicle_count} vehicles"
    return "Failed to get vehicle info"
```

3. **The bot will now use it automatically** when relevant!

### Changing Personality

Edit the `SYSTEM_PROMPT` variable to modify:
- Tone and style
- Knowledge domain
- Response format
- Behavioral guidelines
- In-universe lore

Example: Make it more serious and professional:

```python
SYSTEM_PROMPT = """You are FICSIT Production Assistant, a professional AI system..."""
```

### Per-Player Customization

You can extend the bot to remember player preferences:

```python
self.player_preferences = {}  # Store preferences
# Example: preferred units, language, notification settings
```

## Troubleshooting

### Bot doesn't respond to messages

**Check:**
- FRM auth token is correct
- OpenAI API key is valid
- Both tokens are set in the config
- Server is running and accessible
- Bot shows "Subscribed to getChatMessages"

**Logs will show:**
```
ERROR: Cannot send message: No auth token
```

### OpenAI API Errors

**Common issues:**

1. **Rate limits**: Wait a moment, try again
2. **Invalid API key**: Verify key at platform.openai.com
3. **Insufficient credits**: Add credits to your OpenAI account
4. **Model not available**: Check model name spelling

**Check logs for details:**
```
ERROR: Error getting AI response: ...
```

### Bot responds too slowly

**Optimize:**
- Use `gpt-4o-mini` instead of `gpt-4o` (10x faster)
- Reduce `max_conversation_history` to decrease tokens
- Check your network latency to OpenAI

### High API Costs

**Cost reduction tips:**
- Use `gpt-4o-mini` (very affordable)
- Reduce `max_conversation_history`
- Lower `max_tokens` parameter (default: 300)
- Set up spending limits in OpenAI dashboard

**Typical costs with gpt-4o-mini:**
- 100 conversations: ~$0.05
- 1,000 conversations: ~$0.50

### Connection Issues

**WebSocket disconnects:**
- Check FRM server is running: `/frm http start`
- Verify port is correct (default: 8080)
- Check firewall allows the port
- Bot will auto-reconnect every 5 seconds

**HTTP errors:**
- 401: Auth token invalid
- 404: Endpoint not found (check FRM version)
- 500: Server error (check FRM logs)

## Advanced Features

### Concurrent Tool Calls

The bot can execute multiple tools in parallel:

```python
# Bot intelligently calls multiple tools at once when needed
Player: "Give me a full status report"
Bot: *calls get_power_production, get_player_info, and get_factory_stats simultaneously*
```

### Conversation Threading

Each player has isolated conversation history:
- Player A and Player B have separate contexts
- Bot remembers individual conversations
- No cross-contamination between players

### Graceful Degradation

If OpenAI API is unavailable:
- Bot continues running
- Returns error message to player
- Automatically retries on next message
- No crash or downtime

## Best Practices

### API Key Security

**DO:**
- Keep API keys private
- Use environment variables in production
- Rotate keys regularly
- Set spending limits in OpenAI dashboard

**DON'T:**
- Commit API keys to git
- Share keys publicly
- Use personal keys on public servers

### Production Deployment

For running 24/7:

```bash
# Use a process manager like systemd or supervisor
# Example systemd service:

[Unit]
Description=FICSIT-AI Bot
After=network.target

[Service]
Type=simple
User=satisfactory
WorkingDirectory=/path/to/FicsitRemoteMonitoring
ExecStart=/usr/bin/python3 example_bot.py
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Or use Docker:

```dockerfile
FROM python:3.11-slim
WORKDIR /app
COPY requirements.txt .
RUN pip install -r requirements.txt
COPY example_bot.py .
CMD ["python", "example_bot.py"]
```

### Monitoring

Add logging to track:
- Message count per player
- API costs (tokens used)
- Tool call frequency
- Error rates
- Response times

```python
logger.info(f"Player: {player}, Tokens: {response.usage.total_tokens}")
```

## Examples

### Example Interactions

**Factory Management:**
```
Player: Is my power stable?
Bot: Let me check... *calls get_power_production* Your grid is solid! 2.4GW production, 2.1GW consumption. Battery charging at +350MW. You've got headroom!

Player: Any power issues I should worry about?
Bot: Based on your 15% overhead, you're fine unless you're planning major expansion. Want me to keep monitoring?
```

**Player Coordination:**
```
Player: Where's the team?
Bot: *calls get_player_info* Sarah's at your northern outpost (15k, -42k), Mike's at the coal plant (12k, -45k), and you're solo at main base. Want me to ping them?

Player: Yeah, ping them to come to my location
Bot: *calls create_map_ping* Done! Sent a map marker at your position. They should see it now!
```

**Optimization Help:**
```
Player: My factory feels inefficient, help me optimize it
Bot: Alright pioneer, let's diagnose! First - what's your power efficiency looking like? Running at capacity or got room to grow?

Player: Power is fine, it's production that's slow
Bot: Got it. What are you producing? Let's start with your bottleneck - what item are you short on?
```

## Contributing

Want to enhance the bot? Ideas for contributions:

- Add more FRM API tool integrations
- Implement scheduling/reminder features
- Create web dashboard for bot management
- Add Discord/Telegram bridge
- Implement admin commands
- Add factory statistics trending
- Create custom alert system

## Resources

- **FRM Documentation**: https://docs.ficsit.app/ficsitremotemonitoring/latest/
- **OpenAI API Docs**: https://platform.openai.com/docs/
- **OpenAI Function Calling**: https://platform.openai.com/docs/guides/function-calling
- **FRM Mod on ficsit.app**: https://ficsit.app/mod/FicsitRemoteMonitoring

## License

This example bot is provided as-is for educational purposes. Feel free to modify and use it in your projects!

## FAQ

**Q: How much does this cost to run?**
A: With `gpt-4o-mini`, very little! Typical usage: ~$0.50 per 1,000 conversations. Set spending limits in OpenAI dashboard.

**Q: Can multiple players talk to the bot at once?**
A: Yes! The bot handles concurrent conversations and maintains separate context for each player.

**Q: Does the bot remember conversations after restart?**
A: No, conversation history is in-memory. You could add persistence by saving to a database.

**Q: Can I run this on the same machine as my server?**
A: Absolutely! It's lightweight and won't impact server performance.

**Q: What if OpenAI is down?**
A: The bot will continue running and return error messages to players until the API is back.

**Q: Can the bot run commands on my server?**
A: Not directly - it only has access to FRM API endpoints. For safety, it cannot execute arbitrary commands.

**Q: How do I make the bot respond to all messages, not just tool-appropriate ones?**
A: It already does! The bot responds conversationally to everything. Tool calls are automatic when relevant.

## Support

For issues with:
- **The bot itself**: Open an issue on the FRM repo
- **FRM mod**: Check FRM documentation or Discord
- **OpenAI API**: See OpenAI support docs
