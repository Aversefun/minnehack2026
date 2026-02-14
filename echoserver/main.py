import asyncio
from websockets.asyncio.server import serve

async def echo(websocket):
    async for message in websocket:
        await websocket.send(message)

async def main():
    async with serve(echo, "127.0.0.1", 4000) as server:
        await server.serve_forever()

asyncio.run(main())
