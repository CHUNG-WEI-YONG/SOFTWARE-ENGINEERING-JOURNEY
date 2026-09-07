import numpy as np
import matplotlib.pyplot as plt

np.random.seed(43)

X=np.random.rand(100,1)
Y=3*X+4+np.random.randn(100, 1) * 0.5

w=np.random.randn(1,1)
b=np.zeros((1,1))

learning_rate=0.1
epochs=1000
m=len(X)

loss_history=[]
for epoch in range(epochs):
    y_pred=np.dot(X,w)+b

    error=y_pred-Y
    loss=(1/(2*m))*np.sum(error**2)
    loss_history.append(loss)

    dw=(1/m)*np.dot(X.T,error)
    db=(1/m)*np.sum(error)

    w = w - learning_rate * dw
    b = b - learning_rate * db
    if (epoch + 1) % 20 == 0:
        print(f"Epoch [{epoch+1}/{epochs}] - Loss: {loss:.4f}, w: {w[0,0]:.4f}, b: {b[0,0]:.4f}")

print(f"最终学到的参数: w = {w[0, 0]:.4f}, b = {b[0, 0]:.4f}")

# 绘制拟合线与损失曲线
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 4))

# 左图：数据点与拟合出的最佳直线
ax1.scatter(X, Y, color='blue', alpha=0.6, label='真实数据点')
ax1.plot(X, np.dot(X, w) + b, color='red', linewidth=2, label=f'拟合线: y={w[0,0]:.2f}x+{b[0,0]:.2f}')
ax1.set_title("线性回归拟合结果")
ax1.legend()

# 右图：损失随着训练轮数下降的趋势
ax2.plot(range(epochs), loss_history, color='green', linewidth=2)
ax2.set_title("损失下降曲线 (Cost J over Epochs)")
ax2.set_xlabel("Epoch")
ax2.set_ylabel("Loss")

plt.tight_layout()
plt.show()
