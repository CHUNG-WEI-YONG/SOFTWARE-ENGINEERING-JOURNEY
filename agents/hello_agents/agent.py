from abc import ABC, abstractmethod
from typing import Optional, Any
from .message import Message
from .MyAgents import HelloAgent
from .Config import Config

class Agent(ABC):
    def __init__(self,name:str,llm:HelloAgent,config:Config,system_prompt:str):
        self.name=name;
        self.llm=llm
        self.config=config or Config()
        self.system_prompt=system_prompt
        self.history:list[Message]=[]

    @abstractmethod
    def run(self,input_text:str,**kwargs):
        pass

    def append_history(self,message:Message):
        self.history.append(message)

    def clear_history(self):
        self.history.clear()

    def get_history(self):
        return self.history.copy()

    def __str__(self):
        return f"Agent(name={self.name}, provider={self.llm.provider})"
