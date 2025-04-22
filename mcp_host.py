import asyncio
import os
import json
from typing import List, Dict

from mcp_client import MCPClient
import google.generativeai as genai

async def main():
    client = MCPClient()
    await client.connect(command="python", args=["mcp_server.py"])
    tools = await client.list_tools()

    genai.configure(api_key=os.environ["GEMINI_API_KEY"])
    model = genai.GenerativeModel("gemini-2.0-flash")

    tool_names = [tool.name for tool in tools]

    try:
        while True:
            user_input = input("\n질문을 입력하세요 (종료하려면 'exit'): ")
            if user_input.lower() == "exit":
                break

            tool_descriptions = [
                {
                    "name": tool.name,
                    "description": tool.description,
                    "parameters": tool.inputSchema,
                }
                for tool in tools
            ]

            prompt = f"""
당신은 도구 실행이 가능한 AI 비서입니다.
다음은 사용 가능한 도구 목록입니다:
{json.dumps(tool_descriptions, ensure_ascii=False, indent=2)}

사용자의 질문: "{user_input}"

도구를 사용할 경우 다음 JSON 형식으로만 응답하십시오:
{{"tool_call": "tool_name", "arguments": {{"arg1": "value1", ...}}}}

도구 사용이 필요 없을 경우에는 평범한 문장으로 답변하십시오.
"""

            response = model.generate_content(prompt)
            content = response.text.strip()

            # 도구 실행 응답인지 확인
            if content.startswith("{") and '"tool_call"' in content:
                try:
                    tool_data = json.loads(content)
                    tool_name = tool_data["tool_call"]
                    arguments = tool_data.get("arguments", {})

                    print(f"[도구 호출] {tool_name} with {arguments}")
                    result = await client.call_tool(tool_name, arguments)
                    print(f"[도구 응답] {result}")
                except Exception as e:
                    print(f"[도구 호출 실패] {e}")
            else:
                print("응답:", content)

            await asyncio.sleep(0.1)

    finally:
        await client.close()

if __name__ == "__main__":
    asyncio.run(main())
