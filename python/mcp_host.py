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

            # 도구 목록 JSON 설명
            tool_descriptions = [
                {
                    "name": tool.name,
                    "description": tool.description,
                    "parameters": tool.inputSchema,
                }
                for tool in tools
            ]

            # Gemini에게 어떤 도구를 사용할지 판단하도록 요청
            decision_prompt = f"""
당신은 도구 실행이 가능한 AI 비서입니다.

다음은 사용 가능한 도구 목록입니다:
{json.dumps(tool_descriptions, ensure_ascii=False, indent=2)}

사용자의 질문: "{user_input}"

도구를 사용할 경우 다음 JSON 형식으로만 응답하십시오:
{{"tool_call": "tool_name", "arguments": {{"arg1": "value1", ...}}}}

도구 사용이 필요 없을 경우에는 평범한 문장으로 바로 답변하십시오.
"""

            decision_response = model.generate_content(decision_prompt)
            content = decision_response.text.strip()

            if content.startswith("{") and '"tool_call"' in content:
                try:
                    tool_data = json.loads(content)
                    tool_name = tool_data["tool_call"]
                    arguments = tool_data.get("arguments", {})

                    print(f"[도구 호출] {tool_name} with {arguments}")
                    tool_result = await client.call_tool(tool_name, arguments)
                    print(f"[도구 응답] {tool_result}")

                    # 결과를 Gemini에게 넘겨 자연스럽게 답변 생성
                    final_prompt = f"""
사용자의 질문: "{user_input}"
도구 {tool_name}를 호출한 결과: "{tool_result}"
위 정보를 바탕으로 사용자에게 자연스럽게 대답해줘.
"""
                    final_response = model.generate_content(final_prompt)
                    print("✨ 응답:", final_response.text)

                except Exception as e:
                    print(f"[도구 호출 실패] {e}")
            else:
                print("✨ 응답:", content)

            await asyncio.sleep(0.1)

    finally:
        await client.close()

if __name__ == "__main__":
    asyncio.run(main())
