import mysql.connector

class Database:
    def __init__(self):
        self.config={
            'host':'localhost',
            'user':'root',
            'password':'cwy071109',
            'database':'todo_db',
        }

    def _get__connection(self):
        return mysql.connector.connect(**self.config)
    
    def load_tasks(self):
        conn=self._get__connection();
        cursor=conn.cursor()
        cursor.execute("SELECT content ,finished_at , status FROM tasks")
        results=cursor.fetchall()
        cursor.close()
        conn.close()
        return [row[0] for row in results]

    def add_task(self,tasks,time):
        conn=self._get__connection();
        cursor=conn.cursor()
        query="INSERT INTO tasks(content,finished_at,status) VALUES (%s,%s,'Pending')"
        cursor.execute(query,(tasks,time))
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
        
    def mark_finished(self,task):
        conn=self._get__connection();
        cursor=conn.cursor
        query="UPDATE tasks set status='Done' where content=%s"
        cursor.execute(query,(task))
        conn.commit()
        cursor.close()
        conn.close()        
