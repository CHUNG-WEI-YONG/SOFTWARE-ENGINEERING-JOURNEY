import os
import shutil

def classify_file(filename,RULES):
    name=filename.lower()

    for folder,keywords in RULES.items():
        for word in keywords:
            if word in name:
                return folder
    return "OTHERS"

def organise_folder(path,RULES):
    for filename in os.listdir(path):
        file_path=os.path.join(path,filename)

        if os.path.isfile(file_path):
            folder=classify_file(filename,RULES)

            target_folder=os.path.join(path,folder)

            if not os.path.exists(target_folder):
                os.mkdir(target_folder)
            shutil.move(file_path,os.path.join(target_folder,filename))

