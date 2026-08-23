from hello_agents import SimpleAgent, HelloAgentsLLM, ToolRegistry
from hello_agents.tools import MemoryTool, RAGTool

llm=HelloAgentsLLM()

agent =SimpleAgent(name="agent",llm=llm,system_prompt="你是一个有记忆和知识检索能力的AI助手")
tool_registry=ToolRegistry()

memory_tool=MemoryTool(userid="User123")
tool_registry.register_tool(memory_tool)

rag_tool = RAGTool(knowledge_base_path="./knowledge_base")
tool_registry.register_tool(rag_tool)

agent.tool_registry=tool_registry
response=agent.run("你好！请记住我叫张三，我是一名Python开发者")
print(response)
