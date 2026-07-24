import os
from dotenv import load_dotenv
from openai import OpenAI
from typing import Dict, Any, Callable, Optional,List
import re 

REACT_PROMPT_TEMPLATE = """
请注意，你是一个有能力调用外部工具的智能助手。

可用工具如下:
{tools}

请严格按照以下格式进行回应:

Thought: 你的思考过程，用于分析问题、拆解任务和规划下一步行动。
Action: 你决定采取的行动，必须是以下格式之一:
- `{{tool_name}}[{{tool_input}}]`:调用一个可用工具。
- `Finish[最终答案]`:当你认为已经获得最终答案时。
- 当你收集到足够的信息，能够回答用户的最终问题时，你必须在Action:字段后使用 Finish[最终答案] 来输出最终答案。

现在，请开始解决以下问题:
Question: {question}
History: {history}
"""

load_dotenv()


class AgentsLLM:
    def __init__(
        self,
        model=None,
        api_key=None,
        base_url=None,
        timeout=None,
    ):
        # 1. 优先使用传入参数，未传入则读取环境变量 (默认选用 MODEL_ID1)
        self.model = model or os.getenv("MODEL_ID1")
        api_key = api_key or os.getenv("API_KEY1")
        base_url = base_url or os.getenv("BASE_URL1")
        self.timeout = timeout or int(os.getenv("LLM_TIMEOUT", 60))

        if not base_url:
            raise ValueError("❌ BASE_URL 不能为 None！")

        base_url = base_url.rstrip("/")

        # 2. 智能判断：仅针对阿里百炼 (aliyuncs.com) 补全路径，不破坏 aihubmix / openai 等其他中转服务
        if "aliyuncs.com" in base_url and not base_url.endswith("/compatible-mode/v1"):
            base_url = base_url + "/compatible-mode/v1"

        # 3. 校验关键参数
        if not all([self.model, api_key, base_url]):
            raise ValueError("❌ 缺少必要参数！请检查模型配置。")

        # 4. 初始化 OpenAI 客户端
        self.client = OpenAI(
            api_key=api_key, base_url=base_url, timeout=self.timeout
        )

    def think(
        self, messages: List[Dict[str, str]], temperature: float = 0
    ) -> str:
        print(f"🤖 正在调用模型: {self.model}")
        try:
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

                # 提取增量内容
                content = chunk.choices[0].delta.content or ""
                # 实时打印流式输出 (打字机效果)
                print(content, end="", flush=True)
                collected_content.append(content)

            print("")  # 训练/输出结束换行
            return "".join(collected_content)

        except Exception as e:
            print(f"\n❌ 调用 LLM API 时发生错误: {e}")
            return None

class ToolExecutor:
    def __init__(self):
        self.tools:Dict[str,Dict[str,Any]]={}
        self.InsertTool("GetInfo",self.getInfo,"Get all the available tools")

    def getTools(self,name:str):
        return self.tools.get(name,{}).get("func")

    def InsertTool(self,name:str,func:Callable,description:str):
        if self.tools.get(name):
            print(f"The {name } func is being override")
        self.tools[name]={"description":description,"func":func}
        print(f"tool {name} is being registered")

    def getInfo(self):
        return "\n".join([
            f"- {name}: {info['description']}" 
            for name, info in self.tools.items()
        ])


class ReAct_Agent:
    def __init__(self,client:AgentsLLM,tools:ToolExecutor,max_time=5):
        self.client=client
        self.tools=tools
        self.max_steps=max_time
        self.history=[]

    def _parse_output(self, text: str):
        """解析LLM的输出，提取Thought和Action。
        """
        # Thought: 匹配到 Action: 或文本末尾
        thought_match = re.search(r"Thought:\s*(.*?)(?=\nAction:|$)", text, re.DOTALL)
        # Action: 匹配到文本末尾
        action_match = re.search(r"Action:\s*(.*?)$", text, re.DOTALL)
        thought = thought_match.group(1).strip() if thought_match else None
        action = action_match.group(1).strip() if action_match else None
        return thought, action

    def _parse_action(self, action_text: str):
        """解析Action字符串，提取工具名称和输入。
        """
        match = re.match(r"(\w+)\[(.*)\]", action_text, re.DOTALL)
        if match:
            return match.group(1), match.group(2)
        return None, None

    def generate(self,question):
        self.history=[]
        curr_step=0

        while(curr_step<self.max_steps):
            curr_step+=1
            print(f"This is the {curr_step+1} step")

            tools_str=self.tools.getInfo()
            history_str="\n".join(self.history)
            prompt=REACT_PROMPT_TEMPLATE.format(
                tools=tools_str,
                question=question,
                history=history_str
            )

            user_prompt=[{"role":"user", "content":prompt}]

            response=self.client.think(user_prompt)
            if not response:
                print("LLM not return useful information")
                break

            thought ,action =self._parse_output(response)
            if thought:
                pass
                #print(f"Thought: {thought}")

            if not action:
                print(f"Invalid or empty action do")
                break

            if action.startswith("Finish"):
                final_answer = re.match(r"Finish\[(.*)\]", action).group(1)
                print(f"🎉 最终答案: {final_answer}")
                return final_answer
            

            func,argument=self._parse_action(action)
            if not func or not argument:
                print("Invalid function or argument")
                continue

            print(f"🎬 行动: {func}[{argument}]")
            tool_func=self.tools.getTools(func)
            if not tool_func:
                print("Invalid tool function")
                continue
            else:
                observation=tool_func(argument)

            #print(f"Observation:{observation}")
            self.history.append(f"Thought:{thought}")
            self.history.append(f"Action:{func}")
            self.history.append(f"Observation: {observation}")

        print("Reach the max steps")
        return None



    
        

        