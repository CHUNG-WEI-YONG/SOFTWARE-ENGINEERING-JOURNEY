import numpy as np
import matplotlib.pyplot as plt

m = 100
X = np.zeros((m, 3))
X[:, 0] = np.random.rand(m) * 1000       # 特征 1: 量级较大 (0 ~ 1000)
X[:, 1] = np.random.rand(m) * 5          # 特征 2: 量级较小 (0 ~ 5)
X[:, 2] = np.random.rand(m) * 50

true_w = np.array([[2.5], [-1.8], [4.0]])
true_b = 5.0
y=np.dot(X,true_w)+true_b+np.random.randn(m, 1) * 2.0

mean=np.mean(X,axis=0)
sd=np.std(X,axis=0)
z_score_X=(X-mean)/sd

n_features = z_score_X.shape[1]
w = np.zeros((n_features, 1))  # (3, 1) 的权重向量
b = 0.0                        # 标量偏置
learning_rate = 0.1
epochs = 300

loss_history=[]
for epoch in range(epochs):
    y_expected=np.dot(z_score_X,w)+b

    diff=y_expected-y
    loss=(1/2*m)*np.sum(diff**2)
    loss_history.append(loss)

    dw=1/m*np.dot(z_score_X.T,diff)
    db=1/m*np.sum(diff)

    w=w-learning_rate*dw
    b=b-learning_rate*db

print("训练完成！")
print("学到的权重 w (标准化数据上):")
for i, weight in enumerate(w.flatten()):
    print(f"  w{i+1}: {weight:.4f}")
print(f"学到的偏置 b: {b:.4f}")

# 6. 绘制损失下降曲线
plt.figure(figsize=(6, 4))
plt.plot(range(epochs), loss_history, color='blue', linewidth=2)
plt.title("Multivariate Linear Regression - Cost Curve")
plt.xlabel("Epoch")
plt.ylabel("Loss (MSE)")
plt.grid(True)
plt.tight_layout()
plt.show()