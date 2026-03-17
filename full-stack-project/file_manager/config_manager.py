import json
import os

CONFIG_FILES = "rules.json"

def load_rules():
    if not os.path.exists(CONFIG_FILES):
        return {}
    with open(CONFIG_FILES,"r")as f:
        return json.load(f)
    
def save_rules(rules):
    with open(CONFIG_FILES,"w") as f:
        json.dump(rules,f,indent=4)
