from AiClient import AgentsLLM,ToolExecutor,ReAct_Agent
import ast
PLANNER_PROMPT_TEMPLATE = """
你是一个顶级的AI规划专家。你的任务是将用户提出的复杂问题分解成一个由多个简单步骤组成的行动计划。
请确保计划中的每个步骤都是一个独立的、可执行的子任务，并且严格按照逻辑顺序排列。
你的输出必须是一个Python列表，其中每个元素都是一个描述子任务的字符串。

问题: {question}

请严格按照以下格式输出你的计划,```python与```作为前后缀是必要的:
```python
["步骤1", "步骤2", "步骤3", ...]
```
"""

EXECUTOR_PROMPT_TEMPLATE = """
你是一位顶级的AI执行专家。你的任务是严格按照给定的计划，一步步地解决问题。
你将收到原始问题、完整的计划、以及到目前为止已经完成的步骤和结果。
请你专注于解决“当前步骤”，并仅输出该步骤的最终答案，不要输出任何额外的解释或对话。

# 原始问题:
{question}

# 完整计划:
{plan}

# 历史步骤与结果:
{history}

# 当前步骤:
{current_step}

请仅输出针对“当前步骤”的回答:
"""

class Planner:
    def __init__(self,client:AgentsLLM):
        self.client=client

    def plan(self,question)->str:
        prompt=PLANNER_PROMPT_TEMPLATE.format(question=question)
        user_prompt=[{"role":"user","content":prompt}]

        print("--- 正在生成计划 ---")
        response=self.client.think(user_prompt,0.1) or ""

        print(f"✅ 计划已生成:\n{response}")

        try:
            plan_str = response.split("```python")[1].split("```")[0].strip()
            plan=ast.literal_eval(plan_str)
            return plan if isinstance(plan,list) else []
        except (ValueError, SyntaxError, IndexError) as e:
            print(f"❌ 解析计划时出错: {e}")
            print(f"原始响应: {response}")
            return []
        except Exception as e:
            print(f"❌ 解析计划时发生未知错误: {e}")
            return []


class Executor:
    def __init__(self,agent:AgentsLLM):
        self.agent=agent

    def  execute(self,question,plan:list[str])->str:
        history=""
        print("Start to execute planning")
        for index,action in enumerate(plan):
            print(f"\n-> 正在执行步骤 {index+1}/{len(plan)}: {action}")
            prompt=EXECUTOR_PROMPT_TEMPLATE.format(question=question,history=history,plan=plan,current_step=action)
            message_prompt=[{"role":"user","content":prompt}]
            response=self.agent.think(message_prompt)
            history += f"步骤 {index+1}: {action}\n结果: {response}\n\n"
            print(f"{index+1} step done , planning is {plan} , result is {response}")
        final_answer=response
        return final_answer

class PlanAndSolveAgent:
    def __init__(self,agent:AgentsLLM):
        self.agent=agent
        self.planner=Planner(agent)
        self.executor=Executor(agent)

    def run(self,question:str)->str:
        plans=self.planner.plan(question)
        if not plans:
            print("Failed in generate planning")
            return
        print(f"Planning successfull , plan is  {plans}")
        final=self.executor.execute(question,plans)
        print(f"Execute completely the plan ,final result is {final}")

if __name__=="__main__":
    client=AgentsLLM()
    user=PlanAndSolveAgent(client)
    user.run('一个水果店周一卖出了15个苹果。周二卖出的苹果数量是周一的两倍。周三卖出的数量比周二少了5个。请问这三天总共卖出了多少个苹果?')


    

        





        