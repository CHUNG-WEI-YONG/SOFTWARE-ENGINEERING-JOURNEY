from AiClient import AgentsLLM

INITIAL_PROMPT_TEMPLATE = """
你是一位资深的Python程序员。请根据以下要求，编写一个Python函数。
你的代码必须包含完整的函数签名、文档字符串，并遵循PEP 8编码规范。

要求: {task}

请直接输出代码，不要包含任何额外的解释。
"""

REFLECT_PROMPT_TEMPLATE = """
你是一位极其严格的代码评审专家和资深算法工程师，对代码的性能有极致的要求。
你的任务是审查以下Python代码，并专注于找出其在<strong>算法效率</strong>上的主要瓶颈。

# 原始任务:
{task}

# 待审查的代码:
```python
{code}
```

请分析该代码的时间复杂度，并思考是否存在一种<strong>算法上更优</strong>的解决方案来显著提升性能。
如果存在，请清晰地指出当前算法的不足，并提出具体的、可行的改进算法建议（例如，使用筛法替代试除法）。
如果代码在算法层面已经达到最优，才能回答“无需改进”。

请直接输出你的反馈，不要包含任何额外的解释。
"""

REFINE_PROMPT_TEMPLATE = """
你是一位资深的Python程序员。你正在根据一位代码评审专家的反馈来优化你的代码。

# 原始任务:
{task}

# 你上一轮尝试的代码:
{last_code_attempt}
评审员的反馈：
{feedback}

请根据评审员的反馈，生成一个优化后的新版本代码。
你的代码必须包含完整的函数签名、文档字符串，并遵循PEP 8编码规范。
请直接输出优化后的代码，不要包含任何额外的解释。
"""

class Memory:
    def __init__(self):
        self.memory:list[dict[str,any]]=[]

    def add_record(self,type,content):
        content_str={"type":type,"content":content}
        self.memory.append(content_str)
        print(f"Successfully add memory , type is {type}, content is {content}")

    def get_trajectory(self):
        trajectory_part=[]
        for content in self.memory:
            if content["type"]=='execution':
                trajectory_part.append(f"--- 上一轮尝试 (代码) ---\n{content['content']}")
            elif content['type'] == 'reflection':
                trajectory_part.append(f"--- 评审员反馈 ---\n{content['content']}")
        return trajectory_part

    def get_last_execution(self):
        for content in reversed(self.memory):
            if content['type']=='execution':
                return content['content']
        return None

    
class ReflectionAgent:
    def __init__(self,agent:AgentsLLM,max_iteration=3):
        self.agent=agent
        self.memory=Memory()
        self.max_execution=max_iteration

    def run(self,task):
        print(f"\n--- 开始处理任务 ---\n任务: {task}")

        # --- 1. 初始执行 ---
        print("\n--- 正在进行初始尝试 ---")
        prompt=INITIAL_PROMPT_TEMPLATE.format(task=task)
        first_prompt=[{"role":"user","content":prompt}]
        response=self.agent.think(messages=first_prompt)
        self.memory.add_record('execution',response)

        for i in range(self.max_execution):
            print(f"\n--- 第 {i+1}/{self.max_execution} 轮迭代 ---")

            # a. 反思
            print("\n-> 正在进行反思...")
            last_code=self.memory.get_last_execution()
            refine=REFLECT_PROMPT_TEMPLATE.format(task=task,code=self.memory.get_last_execution())
            prompt=[{"role":"user","content":refine}]
            feedback=self.agent.think(prompt)
            self.memory.add_record('reflection',feedback)
            if "无需改进" in feedback:
                print("\n✅ 反思认为代码已无需改进，任务完成。")
                break

            print("\n-> 正在进行优化...")
            refine_rsp=REFINE_PROMPT_TEMPLATE.format(task=task,last_code_attempt=last_code,feedback=feedback)
            refined_code=self._get_llm_response(refine_rsp)
            self.memory.add_record("execution",refined_code)
        final_code = self.memory.get_last_execution()
        print(f"\n--- 任务完成 ---\n最终生成的代码:\n```python\n{final_code}\n```")
        return final_code

            



    def _get_llm_response(self, prompt: str) -> str:
        """一个辅助方法，用于调用LLM并获取完整的流式响应。"""
        messages = [{"role": "user", "content": prompt}]
        response_text = self.agent.think(messages=messages) or ""
        return response_text

agent=AgentsLLM()
user=ReflectionAgent(agent)
problem=input("What is your code problem\n")
user.run(problem)




        

