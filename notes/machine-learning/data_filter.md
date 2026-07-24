# 📚 Machine Learning, Data Structures & Agent Engineering Notes

---

## 1. 🤖 Machine Learning Fundamentals

### 📊 Data Pipeline & Splitting
* **Data Division**: Datasets are typically split into three parts:
  * **Training Set**: Used to train and fit the model parameters.
  * **Validation Set**: Used to tune hyper-parameters and prevent overfitting.
  * **Testing Set**: Held out until the end to evaluate final, unbiased model performance.
* **Class Balance**: 
  * Having an equal number of positive (`1`) and negative (`0`) observations in binary training data is **highly recommended** (though not strictly mandatory) to prevent model bias toward the majority class.
  * Techniques like **Oversampling** (e.g., `RandomOverSampler`) or **Undersampling** are used during preprocessing.

---

### 🔍 Core Algorithms Cheat Sheet

| Algorithm | Model Type | Core Mechanism | Key Features / Notes |
| :--- | :--- | :--- | :--- |
| **K-Nearest Neighbors (KNN)** | Instance-based (Lazy Learner) | Stores data in memory and classifies new points based on a majority vote of the $k$ closest neighbors. | Requires feature scaling (`StandardScaler`). $K=1$ often leads to severe overfitting. |
| **Naïve Bayes** | Probabilistic Classifier | Applies Bayes' Theorem: $$P(Y\Vert{}X) = \frac{P(X\Vert{}Y) \cdot P(Y)}{P(X)}$$ | Assumes all input features are conditionally independent given the class label. |
| **Logistic Regression** | Linear Classification Model | Converts linear outputs ($z = w^Tx + b$) into a $[0, 1]$ probability range using the **Sigmoid function**: $$\sigma(z) = \frac{1}{1 + e^{-z}}$$ | Used for binary classification (unlike Linear Regression, which predicts continuous values). |
| **Perceptron / SVM** | Linear Decision Boundaries | Finds the optimal hyperplane separating classes. Uses polynomial features ($x \to x^2$) or kernel tricks for non-linear data. | Polynomial expansion projects complex non-linear data into a higher dimension where it becomes linearly separable. |

---

### 🛠️ Python Preprocessing & Model Setup

```python
import numpy as np
from sklearn.preprocessing import StandardScaler
from imblearn.over_sampling import RandomOverSampler
from sklearn.neighbors import KNeighborsClassifier
from sklearn.metrics import accuracy_score, classification_report

# 1. Scaler & Oversampler Pipeline Function
def scale_dataset(dataframe, oversample=False):
    x = dataframe[cols[:-1]].values
    y = dataframe[cols[-1]].values

    # Feature Scaling
    scaler = StandardScaler()
    x = scaler.fit_transform(x)

    # Over-sampling Imbalanced Target Class
    if oversample:
        ros = RandomOverSampler()
        x, y = ros.fit_resample(x, y)

    data = np.hstack((x, np.reshape(y, (-1, 1))))
    return data, x, y

# 2. Scaling Train, Validation, and Test Sets
# ONLY training data gets oversampled!
train, X_train, y_train = scale_dataset(train_df, oversample=True)
valid, X_valid, y_valid = scale_dataset(valid_df, oversample=False)
test, X_test, y_test   = scale_dataset(test_df, oversample=False)

# 3. Model Training Example (KNN)
knn_model = KNeighborsClassifier(n_neighbors=3)
knn_model.fit(X_train, y_train)

# Evaluation
y_pred = knn_model.predict(X_valid)
print("Accuracy:", accuracy_score(y_valid, y_pred))
print(classification_report(y_valid, y_pred))