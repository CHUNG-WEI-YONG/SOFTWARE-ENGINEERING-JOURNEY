import numpy as np
from PIL import Image,ImageOps
import os
import urllib.request



def relu(Z):
    return np.maximum(0,Z)

def relu_derivative(Z):
    return (Z > 0).astype(float)

def softmax(Z):
    # 减去每行的最大值，避免 e^z 发生数值溢出 (Numerical Stability)
    exp_Z = np.exp(Z - np.max(Z, axis=1, keepdims=True))
    return exp_Z / np.sum(exp_Z, axis=1, keepdims=True)

def one_hot(Y,num_classes=10):
    m=Y.shape[0]
    one_hot_y=np.zeros((m,num_classes))
    one_hot_y[np.arange(m),Y]=1
    return one_hot_y


class DigitIdentifier:
    def __init__(self,input_dim=784,hidden_dim=128,output_dim=10,lr=0.1):
        self.lr = lr
        
        # He 初始化隐藏层权重
        self.W1 = np.random.randn(input_dim, hidden_dim) * np.sqrt(2.0 / input_dim)
        self.b1 = np.zeros((1, hidden_dim))
        
        # Xavier 初始化输出层权重
        self.W2 = np.random.randn(hidden_dim, output_dim) * np.sqrt(1.0 / hidden_dim)
        self.b2 = np.zeros((1, output_dim))

    def forward(self,X):
        self.X=X
        self.Z1=np.dot(self.X,self.W1 )+self.b1
        self.A1=relu(self.Z1)

        self.Z2=np.dot(self.A1,self.W2)+self.b2
        self.A2=softmax(self.Z2)
        return self.A2

    def backward(self, Y_one_hot):
        m = self.X.shape[0]

        # 输出层梯度：Softmax + 交叉熵求导直接化简为 (预测值 - 真实值)
        dZ2 = self.A2 - Y_one_hot                               # (m, 10)
        dW2 = (1 / m) * np.dot(self.A1.T, dZ2)                  # (128, 10)
        db2 = (1 / m) * np.sum(dZ2, axis=0, keepdims=True)      # (1, 10)

        # 误差穿透到隐藏层
        dA1 = np.dot(dZ2, self.W2.T)                            # (m, 128)
        dZ1 = dA1 * relu_derivative(self.Z1)                    # (m, 128)
        dW1 = (1 / m) * np.dot(self.X.T, dZ1)                   # (784, 128)
        db1 = (1 / m) * np.sum(dZ1, axis=0, keepdims=True)      # (1, 128)

        # 梯度更新 (SGD)
        self.W1 -= self.lr * dW1
        self.b1 -= self.lr * db1
        self.W2 -= self.lr * dW2
        self.b2 -= self.lr * db2

    def train(self,X,Y,epochs=100,batch_size=64):
        m=Y.shape[0]
        Y_one_hot=one_hot(Y,10)

        for epoch in range (epochs):
            indices = np.random.permutation(m)
            X_shuffled = X[indices]
            Y_shuffled = Y_one_hot[indices]

            for i in range(0,m,batch_size):
                X_batch = X_shuffled[i:i + batch_size]
                Y_batch = Y_shuffled[i:i + batch_size]

                self.forward(X_batch)
                self.backward(Y_batch)

            if (epoch + 1) % 10 == 0 or epoch == 0:
                probs = self.forward(X)
                loss = -np.mean(np.sum(Y_one_hot * np.log(probs + 1e-15), axis=1))
                preds = np.argmax(probs, axis=1)
                acc = np.mean(preds == Y) * 100
                print(f"Epoch {epoch + 1:3d} | Loss: {loss:.4f} | Accuracy: {acc:.2f}%")

    def predict(self,X):
        probs=self.forward(X)
        return np.argmax(probs,axis=1)


def predict_my_image(image_path, model):
    # 1. 打开图片并转为灰度 (L 通道)
    img = Image.open(image_path).convert('L')
    
    # 2. 缩放到 MNIST 标准尺寸 28x28
    img = img.resize((28, 28))
    
    # 3. 检查并反色：
    # 多数画图软件是白底黑字，而 MNIST 是黑底白字
    # 如果四个角的像素接近白色(>128)，说明是白底，需要反相
    img_array = np.array(img)
    if img_array[0, 0] > 128:
        img = ImageOps.invert(img)
        img_array = np.array(img)

    # 4. 展平成 784 维向量并归一化到 [0, 1]
    x_input = img_array.reshape(1, 784).astype(np.float32) / 255.0

    # 5. 送进你写好的模型前向传播
    prediction = model.predict(x_input)[0]
    
    # 获取输出层的 10 个概率分布
    probabilities = model.forward(x_input)[0]
    confidence = probabilities[prediction] * 100

    print(f"\n[自定义图片识别结果]")
    print(f"-> 预测数字: {prediction}")
    print(f"-> 置信度: {confidence:.2f}%")
    return prediction


if __name__ =="__main__":
    url = "https://storage.googleapis.com/tensorflow/tf-keras-datasets/mnist.npz"
    file_path = "mnist.npz"

    if not os.path.exists(file_path):
        print("正在下载轻量 mnist.npz (约 11MB)...")
        urllib.request.urlretrieve(url, file_path)
        print("下载完成！")

    with np.load(file_path) as data:
        X_train_raw = data['x_train']
        y_train = data['y_train']
        X_test_raw = data['x_test']
        y_test = data['y_test']

# 3. 预处理成 784 维向量
    X_train = X_train_raw.reshape(-1, 784).astype(np.float32) / 255.0
    X_test = X_test_raw.reshape(-1, 784).astype(np.float32) / 255.0

    print("数据读取成功！训练集形状:", X_train.shape)

    machine=DigitIdentifier()
    machine.train(X_train,y_train)
    result=machine.predict(X_test)
    acc = np.mean(result == y_test) * 100
    print(f"\n==========================================")
    print(f"测试集最终识别准确率: {acc:.2f}%")
    print(f"==========================================")
    print("前 10 张测试图片真实标签:", y_test[:10])
    print("前 10 张测试图片模型预测:", result[:10])
    