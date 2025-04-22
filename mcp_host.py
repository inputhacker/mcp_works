import asyncio
import os
from typing import List, Dict

from mcp_client import MCPClient
import google.generativeai as genai

async def main():
    # MCP 클라이언트 인스턴스 생성
    client = MCPClient()
    await client.connect(command="python", args=["mcp_server.py"])
    tools = await client.list_tools()

    # Gemini 설정
    genai.configure(api_key=os.environ["GEMINI_API_KEY"])
    model = genai.GenerativeModel("gemini-2.0-flash")
    #model = genai.GenerativeModel("gemini-2.5-pro")

    # 사용자 입력 처리
    while True:
        user_input = input("\n질문을 입력하세요 (종료하려면 'exit'): ")
        if user_input.lower() == "exit":
            break

        # Gemini에게 도구 목록과 함께 사용자 입력 전달
        tool_descriptions = [
            {
                "name": tool.name,
                "description": tool.description,
                "parameters": tool.inputSchema,
            }
            for tool in tools
        ]

        prompt = f"""
사용자의 질문: "{user_input}"
다음은 사용 가능한 도구 목록입니다:
{tool_descriptions}
사용자의 질문에 적합한 도구가 있다면 해당 도구를 호출하고, 없다면 직접 응답을 생성하세요.
"""

        response = model.generate_content(prompt)
        print("응답:", response.text)
        await asyncio.sleep(0.1)
    await client.close()

if __name__ == "__main__":
    asyncio.run(main())

