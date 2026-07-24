import os
import re
from openai import OpenAI
from AiClient import AgentsLLM

# 建议：实际开发中优先使用环境变量读取，避免硬编码
API_KEY = "sk-x7R478mlicWKQIaD17C4B02c04C5400486C1AfD860145972"
BASE_URL = "https://aihubmix.com/v1"
MODEL_ID = "coding-glm-4.7-free"

AGENT_SYSTEM_PROMPT = """你是 calculator helper。你的任务是分析用户的请求，并使用可用工具一步步地解决问题。

# 可用工具:
add(a, b)
substract(a, b)
multiply(a, b)
divide(a, b)

# 输出格式要求:
你的每次回复必须严格遵循以下格式，包含一对 Thought 和 Action：

Thought: [你的思考过程和下一步计划]
Action: [你要执行的具体行动]

Action 的格式必须是以下之一：
1. 调用工具：function_name(a=数字, b=数字)   <-- ⚠️ 参数名必须是 a 和 b！
2. 结束任务：Finish[最终答案]

# 示例:
Thought: 我需要计算 10 乘以 5
Action: multiply(a=10, b=5)

请开始吧！
"""

def add(a, b):
    return a + b

def substract(a, b):
    return a - b

def multiply(a, b):
    return a * b

def divide(a, b):
    return a / b

tools = {
    "add": add,
    "substract": substract,
    "multiply": multiply,
    "divide": divide
}

class OpenAIClient:
    def __init__(self, model, api, url):
        self.model = model
        self.agent = OpenAI(api_key=api, base_url=url)

    def generate(self, message, system_prompt):
        prompt_message = [
            {"role": "system", "content": system_prompt},
            {"role": "user", "content": message}
        ]
        response = self.agent.chat.completions.create(
            model=self.model,
            messages=prompt_message,
            temperature=0.1
        )
        answer = response.choices[0].message.content or ""
        return answer


user = AgentsLLM()
expression = input("Enter expression: ")
request = f"calculate expression: {expression}"
prompt_history = [f"User prompt: {request}"]

for i in range(5):
    print(f"\n--- This is the {i+1} round ---")
    full_prompt = "\n".join(prompt_history)
    prompt_message = [
            {"role": "system", "content": AGENT_SYSTEM_PROMPT},
            {"role": "user", "content": full_prompt}
        ]
    response = user.think(prompt_message,0.1)
    print(response)
    prompt_history.append(response)

    # 🎯 检查一：如果触发 Finish，打印答案并立即终止循环
    finish_match = re.search(r"Action:\s*Finish\[(.*?)\]", response)
    if finish_match:
        final_answer = finish_match.group(1)
        print(f"\n🎉 Final answer is: {final_answer}")
        break  # 👈 核心绝杀：必须 break！

    # 🎯 检查二：解析工具调用
    tool_match = re.search(r"Action:\s*(\w+)\((.*?)\)", response)
    if tool_match:
        func_name = tool_match.group(1)
        args_str = tool_match.group(2)
        
        # 提取形如 a=10, b=20 的参数
        kwargs = {}
        for arg in args_str.split(','):
            if '=' in arg:
                k, v = arg.split('=')
                kwargs[k.strip()] = float(v.strip().strip('"\''))

        # 执行工具
        if func_name in tools:
            try:
                result = tools[func_name](**kwargs)
                # 浮点数处理
                if isinstance(result, float) and result.is_integer():
                    result = int(result)
                observation = f"Observation: {result}"
            except Exception as e:
                observation = f"Observation: Error executing {func_name}: {str(e)}"
        else:
            observation = f"Observation: Tool '{func_name}' not found."
    else:
        observation = "Observation: Invalid Action format. Please use function_name(a=num, b=num) or Finish[answer]."

    print(observation)
    # 🎯 将格式规范的 Observation 追加进对话历史
    prompt_history.append(observation)

print("\nAgent ended")










