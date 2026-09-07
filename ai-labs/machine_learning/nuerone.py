import numpy as np

class SimpleNeurone:
    def __init__(self,n_inputs,activation='sigmoid'):
        self.w=np.random.randn(n_inputs,1)*0.01
        self.b=0.0
        self.activation=activation

    def _activate(self,z):
        if  self.activation=='linear':
            return z
        elif self.activation=='sigmoid':
            z = np.clip(z, -500, 500)
            return 1.0 / (1.0 + np.exp(-z))
        elif self.activation == 'relu':
            return np.maximum(0, z)
        else:
            raise ValueError(f"未知的激活函数: {self.activation}")

    def forward(self,X):
        self.z=np.dot(X,self.w)+self.b
        self.a=self._activate(self.z)
        return self.a

class DenseLayer:
    def __init__(self,n_inputs,n_units,activation='relu'):
        self.w=np.random.randn(n_inputs,n_units)*0.01
        self.b=np.zeros((1,n_units))
        self.activation=activation

    def _activate(self,z):
        if self.activation == 'linear':
            return z
        elif self.activation == 'sigmoid':
            z = np.clip(z, -500, 500)
            return 1.0 / (1.0 + np.exp(-z))
        elif self.activation == 'relu':
            return np.maximum(0, z)
        else:
            raise ValueError(f"未知的激活函数: {self.activation}")

    def forward(self,x):
        self.z=np.dot(x,self.w)+self.b
        self.a=self._activate(self.z)
        return self.a

    def _activate_derivative(self,z):
        if self.activation=='sigmoid':
            s=self._activate(z)
            return s*(1.0-s)
        elif self.activation=='linear':
            return np.ones_like(z)

        elif self.activation == 'relu':
            return (z > 0).astype(float)

    def _backward(self,da):
        m=self.input.shape[0]
        dz=da*self._activate_derivative(self.z)

        self.dw=(1/m)*np.dot(self.input.T,dz)
        self.db=(1 / m) * np.sum(dz, axis=0, keepdims=True)

        da_prev = np.dot(dz, self.w.T)
        return da_prev

    def update(self, lr):
        """根据算出来的梯度改正参数 (梯度下降)"""
        self.w -= lr * self.dw
        self.b -= lr * self.db

        


class NeuroNetwork:
    def __init__(self):
        self.layers=[]

    def add(self,layer:DenseLayer):
        self.layers.append(layer)

    def forward(self,X):
        a=X

        for i , layer in enumerate(self.layers):
            a=layer.forward(a)

        return a 

        



if __name__=="__main__":
    X_sample = np.array([
        [1.2, 0.5, -1.0, 2.1],
        [0.1, -0.8, 1.5, 0.4],
        [3.0, 1.1, -0.2, -1.5]
    ])
    print("=== 1. 单层 DenseLayer (包含 3 个神经元，ReLU 激活) ===")
    # 4 个特征输入 -> 映射为 3 个神经元输出
    dense1 = DenseLayer(n_inputs=4, n_units=3, activation='relu')
    a1 = dense1.forward(X_sample)
    print("Layer 1 输出形状 (样本数, 神经元数):", a1.shape)
    print("Layer 1 输出结果 (全为非负数):\n", a1)

    print("\n=== 2. 完整 2 层网络前向传播 (Hidden: ReLU -> Output: Sigmoid) ===")
    model = NeuroNetwork()
    # 隐藏层：4 输入 -> 3 个隐层神经元 (ReLU 提取特征)
    model.add(DenseLayer(n_inputs=4, n_units=3, activation='relu'))
    # 输出层：3 个隐层输出 -> 1 个输出神经元 (Sigmoid 输出 0~1 的预测概率)
    model.add(DenseLayer(n_inputs=3, n_units=1, activation='sigmoid'))

    final_prob = model.forward(X_sample)
    print("最终预测输出形状:", final_prob.shape)
    print("3 个样本对应的预测概率值:\n", final_prob)

        