from mcp.server.fastmcp import FastMCP
from datetime import datetime

# MCP 서버 인스턴스 생성
mcp = FastMCP("LocalToolServer")

# 현재 시간을 반환하는 도구
@mcp.tool()
def time_server() -> str:
    """현재 시간을 ISO 형식으로 반환합니다."""
    return datetime.now().isoformat()

# 입력된 문자열을 반전시키는 도구
@mcp.tool()
def reverse_server(text: str) -> str:
    """입력된 문자열을 반전시킵니다."""
    return text[::-1]

# 서버 실행
if __name__ == "__main__":
    mcp.run(transport="stdio")
