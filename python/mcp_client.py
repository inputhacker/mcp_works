import asyncio
from contextlib import AsyncExitStack
from typing import Optional, Dict, Any, List

from mcp import ClientSession
from mcp.client.stdio import stdio_client
from mcp import StdioServerParameters

class MCPClient:
    def __init__(self):
        self.exit_stack = AsyncExitStack()
        self.session: Optional[ClientSession] = None

    async def connect(self, command: str, args: List[str]):
        # MCP 서버에 연결
        server_params = StdioServerParameters(command=command, args=args)
        stdio = await self.exit_stack.enter_async_context(stdio_client(server_params))
        self.session = await self.exit_stack.enter_async_context(ClientSession(*stdio))
        await self.session.initialize()
        print("서버에 연결되었습니다.")

    async def list_tools(self):
        # 도구 목록 가져오기
        response = await self.session.list_tools()
        return response.tools

    async def call_tool(self, tool_name: str, args: Dict[str, Any]):
        # 도구 호출
        result = await self.session.call_tool(tool_name, args)
        return result.content

    async def close(self):
        # 리소스 정리
        await self.exit_stack.aclose()

