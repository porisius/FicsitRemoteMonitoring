# FicsitRemoteMonitoring Chat Bot Example

A performant, async Python bot that interacts with the FicsitRemoteMonitoring mod's HTTP/WebSocket server to receive and respond to player messages in Satisfactory.

## Features

- **Async/Await Architecture**: Built with `asyncio` and `aiohttp` for high performance
- **WebSocket Integration**: Real-time message reception via WebSocket subscriptions
- **Command System**: Responds to player commands starting with `!`
- **Auto-Reconnection**: Automatically reconnects if the connection drops
- **Duplicate Prevention**: Ensures messages are only processed once
- **Colored Messages**: Sends messages with custom colors
- **Extensible**: Easy to add new commands and features

## Prerequisites

1. **Satisfactory Dedicated Server** with FicsitRemoteMonitoring mod installed
2. **Python 3.7+**
3. **pip** for installing dependencies

## Installation

1. Install Python dependencies:
```bash
pip install -r requirements.txt
```

2. Start your Satisfactory server and enable the FRM web server:
   - In-game or via RCON: `/frm http start`
   - Or set `Web_Autostart = true` in the config file

3. Get your authentication token:
   - Location: `<SatisfactoryServer>/FactoryGame/Configs/FicsitRemoteMonitoring/WebServer.cfg`
   - Look for the `Authentication_Token` field
   - If empty, start the server once and it will auto-generate

4. Edit `example_bot.py` and set your configuration:
```python
AUTH_TOKEN = "your_token_here"  # Required for sending messages
HOST = "localhost"              # Change if server is remote
PORT = 8080                     # Default FRM port
BOT_NAME = "FRM Bot"           # Your bot's display name
```

## Usage

Run the bot:
```bash
python example_bot.py
```

The bot will:
1. Connect to the WebSocket server
2. Subscribe to chat messages
3. Listen for player commands
4. Respond automatically

## Available Commands

Players can interact with the bot using these commands in-game:

- `!help` - Show available commands
- `!echo <text>` - Bot echoes back the text
- `!time` - Show current server time
- `!ping` - Simple ping/pong response
- `!reverse <text>` - Reverse the provided text

## Adding Custom Commands

To add a new command, edit the `handle_command` method in `example_bot.py`:

```python
async def handle_command(self, sender: str, message: str):
    parts = message.split(maxsplit=1)
    command = parts[0].lower()
    args = parts[1] if len(parts) > 1 else ""

    # Add your custom command here
    if command == "!mycommand":
        await self.send_chat_message(f"Custom response for {sender}")
```

## Architecture

### WebSocket Subscription
The bot connects via WebSocket and subscribes to the `getChatMessages` endpoint:
```json
{
  "action": "subscribe",
  "endpoints": "getChatMessages"
}
```

### Message Format (Received)
Messages from players arrive in this format:
```json
{
  "endpoint": "getChatMessages",
  "data": [
    {
      "ServerTimeStamp": 1002.81,
      "TimeStamp": 1234567890,
      "Sender": "PlayerName",
      "Type": "Player",
      "Message": "!help",
      "Color": { "R": 0.0, "G": 0.39, "B": 0.25, "A": 1.0 }
    }
  ]
}
```

### Sending Messages
Messages are sent via HTTP POST to `/api/sendChatMessage`:
```python
await self.send_chat_message("Hello players!")
```

This sends:
```json
{
  "message": "Hello players!",
  "sender": "FRM Bot",
  "color": { "r": 0.2, "g": 0.8, "b": 1.0, "a": 1.0 }
}
```

## Performance Features

- **Async I/O**: All network operations are non-blocking
- **Connection Pooling**: `aiohttp` automatically pools connections
- **Efficient Deduplication**: Set-based message tracking prevents reprocessing
- **Auto-Reconnect**: Resilient to network interruptions
- **Minimal Memory**: Only stores message IDs, not full message history

## Troubleshooting

### Bot receives messages but can't send responses
- Check that `AUTH_TOKEN` is set correctly
- Verify token matches the one in `WebServer.cfg`
- Ensure the token hasn't changed (it regenerates if the file is deleted)

### Connection refused
- Verify the FRM web server is running: `/frm http start`
- Check the port matches your configuration (default: 8080)
- If connecting remotely, ensure firewall allows the port

### Bot doesn't respond to commands
- Commands must start with `!`
- Only "Player" type messages are processed (not System/Ada)
- Check the logs for errors
- Verify authentication is working for sending messages

### WebSocket disconnects frequently
- Check server logs for errors
- Ensure server is stable and not restarting
- The bot will auto-reconnect every 5 seconds

## Advanced Usage

### Multiple Endpoint Subscriptions
Subscribe to multiple data streams:
```python
subscribe_msg = {
    "action": "subscribe",
    "endpoints": ["getChatMessages", "getPlayer", "getPower"]
}
await self.ws.send_json(subscribe_msg)
```

### Custom Message Processing
Override `handle_message` for custom behavior:
```python
async def handle_message(self, msg_data: Dict):
    # Custom processing logic
    await super().handle_message(msg_data)
```

### Background Tasks
Run periodic tasks alongside the bot:
```python
async def periodic_announcement():
    while True:
        await asyncio.sleep(300)  # Every 5 minutes
        await bot.send_chat_message("Server tip: Use !help for commands")

async with FRMBot(...) as bot:
    # Run bot and periodic task concurrently
    await asyncio.gather(
        bot.run(),
        periodic_announcement()
    )
```

## API Reference

See the full FRM API documentation at:
- https://docs.ficsit.app/ficsitremotemonitoring/latest/

Key endpoints used by this bot:
- WebSocket: `ws://host:port/`
- Send Chat: `POST /api/sendChatMessage`
- Get Chat: `GET /api/getChatMessages`

## License

This example bot is provided as-is for educational purposes. Feel free to modify and use it in your projects.

## Contributing

Contributions are welcome! Some ideas for enhancements:
- Database integration for persistent commands
- Web dashboard for bot management
- Integration with Discord/Telegram
- Game state monitoring and alerts
- Custom scripting language for users
- Rate limiting and spam prevention
