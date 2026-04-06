import mysql.connector

class Database:
    def __init__(self):
        self.config={
            'host':'localhost',
            'user':'root',
            'password':'-',
            'database':'todo_db',
        }

    def _get__connection(self):
        return mysql.connector.connect(**self.config)
    
    def load_tasks(self):
        conn=self._get__connection();
        cursor=conn.cursor()
        cursor.execute("SELECT content FROM tasks")
        results=cursor.fetchall()
        cursor.close()
        conn.close()
        return [row[0] for row in results]

    def add_task(self,tasks):
        conn=self._get__connection();
        cursor=conn.cursor()
        query="INSERT INTO tasks(content) VALUES (%s)"
        cursor.execute(query,(tasks,))
        conn.commit()
        cursor.close()
        conn.close()

    def delete_task(self,task):
        conn=self._get__connection()
        cursor=conn.cursor()
        query="DELETE from tasks when content=%s"
        cursor.execute(query,(task,))
        conn.commit()
        cursor.close()
        conn.close()
        
        
