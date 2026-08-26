import os
import re
import math
from typing import TypedDict, List, Dict, Optional, Callable, Any
from langgraph.graph import StateGraph, START, END
from langgraph.checkpoint.memory import MemorySaver
import arxiv
import requests
from dotenv import load_dotenv
from openai import OpenAI


load_dotenv()

class AgentsLLM:
    def __init__(
        self,
        model: Optional[str] = None,
        api_key: Optional[str] = None,
        base_url: Optional[str] = None,
        timeout: int = 60,
    ):
        raw_key = api_key or os.getenv("API_KEY") or os.getenv("DASHSCOPE_API_KEY") or ""
        raw_url = base_url or os.getenv("BASE_URL") or ""
        raw_model = model or os.getenv("MODEL_ID", "qwen-max")

        # 核心修复：强力去除换行符、回车符和首尾空格
        self.api_key = str(raw_key).strip().replace("\n", "").replace("\r", "")
        self.base_url = str(raw_url).strip().replace("\n", "").replace("\r", "").rstrip("/")
        self.model = str(raw_model).strip().replace("\n", "").replace("\r", "")
        self.timeout = int(os.getenv("LLM_TIMEOUT", timeout))

        # 2. 校验关键参数
        if not self.api_key:
            raise ValueError("❌ 未检测到 API_KEY / DASHSCOPE_API_KEY，请检查 .env 文件！")
        if not self.base_url:
            raise ValueError("❌ 未检测到 BASE_URL，请检查 .env 文件！")

        self.base_url = self.base_url.rstrip("/")
        # 针对阿里云百炼 endpoint 兼容性处理
        if "aliyuncs.com" in self.base_url and not self.base_url.endswith("/compatible-mode/v1"):
            self.base_url += "/compatible-mode/v1"

        # 3. 初始化 OpenAI 客户端
        self.client = OpenAI(
            api_key=self.api_key,
            base_url=self.base_url,
            timeout=self.timeout
        )

    def think(
        self,
        messages: List[Dict[str, str]],
        temperature: float = 0.1,
        stream: bool = False
    ) -> str:
        """
        调用 LLM 进行思考与回复
        :param messages: OpenAI 格式的 messages 列表
        :param temperature: 采样温度 (默认 0.1 保证输出稳定性)
        :param stream: 是否在终端开启打字机流式输出
        """
        try:
            if stream:
                print(f"🤖 [Calling {self.model}]... ", end="", flush=True)
                response = self.client.chat.completions.create(
                    model=self.model,
                    messages=messages,
                    temperature=temperature,
                    stream=True,
                )
                collected_content = []
                for chunk in response:
                    if not chunk.choices:
                        continue
                    delta = chunk.choices[0].delta.content or ""
                    print(delta, end="", flush=True)
                    collected_content.append(delta)
                print("")
                return "".join(collected_content)
            else:
                response = self.client.chat.completions.create(
                    model=self.model,
                    messages=messages,
                    temperature=temperature,
                    stream=False,
                )
                return response.choices[0].message.content or ""

        except Exception as e:
            print(f"\n❌ 调用 LLM API 时发生错误: {e}")
            return ""

    def get_embedding(self, text: str) -> List[float]:
        """
        获取文本的向量表示 (使用 .env 中配置的 text-embedding-v3)
        """
        embed_key = os.getenv("EMBED_API_KEY") or self.api_key
        embed_base_url = os.getenv("EMBED_BASE_URL", "https://dashscope.aliyuncs.com/compatible-mode/v1").rstrip("/")
        embed_model = os.getenv("EMBED_MODEL_NAME", "text-embedding-v3")

        try:
            embed_client = OpenAI(api_key=embed_key, base_url=embed_base_url)
            resp = embed_client.embeddings.create(
                model=embed_model,
                input=text
            )
            return resp.data[0].embedding
        except Exception as e:
            print(f"❌ 获取 Embedding 失败: {e}")
            return []

# =============================================================
# 1. 状态定义
# =============================================================


SEARCH_REACT_PROMPT = """You are a professional academic research retrieval agent.
Your task is to analyze the user's research topic, break it down into 2-3 formal frontier academic search terms, and call retrieval tools to fetch the most relevant top-tier conference/journal papers.

Available Tools:
{tools}

Output Format Guidelines:
- Thought: Analyze the academic keywords and retrieval strategy.
- Action: `tool_name[query_text]` (e.g., `search_semantic[KV Cache Quantization]` or `search_arxiv[Speculative Decoding]`)
  * Note: Pass only the raw query string inside the brackets. Do not include parameter names like 'query=' or quotes.
- When you have gathered sufficient literature, you must output: `Action: Finish[retrieval complete]`

Question: {question}
History:
{history}
"""

REACT_PROMPT_TEMPLATE = """You are a professional academic retrieval agent. Your goal is to retrieve high-quality, state-of-the-art research papers for a given research topic using the available tools.

Available Tools:
{tools}

Strictly follow the Thought -> Action -> Observation cycle:

Question: The research topic to investigate
Thought: Reason about the next step to take (e.g., formulate precise English search keywords, select the appropriate tool)
Action: tool_name[argument]
Observation: The tool execution result
... (Repeat Thought / Action / Observation as needed)
Thought: I have collected sufficient literature data and am ready to conclude.
Action: Finish[retrieval complete]

Current Task:
Question: {question}

Action History:
{history}
"""

class ToolExecutor:
    def __init__(self):
        self.tools: Dict[str, Dict[str, Any]] = {}
        self.register_tool("GetInfo", self.getInfo, "获取所有当前可用的工具列表及其描述")

    def get_tool(self, name: str) -> Optional[Callable]:
        return self.tools.get(name, {}).get("func")

    def register_tool(self, name: str, func: Callable, description: str):
        self.tools[name] = {"description": description, "func": func}

    def InsertTool(self, name: str, func: Callable, description: str):
        self.register_tool(name, func, description)

    def getTools(self, name: str) -> Optional[Callable]:
        return self.get_tool(name)

    def getInfo(self) -> str:
        return "\n".join([
            f"- `{name}`: {info['description']}" 
            for name, info in self.tools.items()
        ])


# =============================================================
# 4. 标准 ReAct Agent 基类（包含解析器与状态管理）
# =============================================================
class ReAct_Agent:
    def __init__(
        self,
        client: AgentsLLM,
        tools: ToolExecutor,
        prompt: str,
        max_time: int = 4
    ):
        self.client = client
        self.tools = tools
        self.prompt = prompt
        self.max_steps = max_time
        self.history: List[str] = []

    def _parse_output(self, text: str):
        """解析 LLM 的 Thought 和 Action"""
        thought_match = re.search(r"Thought:\s*(.*?)(?=\nAction:|$)", text, re.DOTALL)
        action_match = re.search(r"Action:\s*(.*?)$", text, re.DOTALL)
        thought = thought_match.group(1).strip() if thought_match else ""
        action = action_match.group(1).strip() if action_match else ""
        return thought, action

    def _parse_action(self, action_text: str):
        """解析 Action 文本，提取工具名和输入参数"""
        match = re.match(r"(\w+)\[(.*)\]", action_text, re.DOTALL)
        if match:
            return match.group(1).strip(), match.group(2).strip()
        return None, None








    
        

        