# Basic Neural Network Implementation from Scratch

## Project Overview
This project demystifies the mathematical foundations and mechanics of deep learning by constructing a fully functional multi-layer perceptron (MLP) from first principles. Built strictly with standard Python and NumPy, it implements core primitives—tensor contractions, multivariable chain-rule backpropagation, batch shuffling, and weight updates—without relying on autograd engines or high-level frameworks (such as PyTorch, TensorFlow, or JAX).

---

## Network Architecture
The model implements a 2-layer feedforward neural network designed for single-channel image classification:

* **Input Layer (`X`)**: 784 dimensions (flattened $28 \times 28$ grayscale pixel arrays normalized to the floating-point range $[0.0, 1.0]$).
* **Hidden Layer (`Z1`, `A1`)**: 128 hidden neurons.
  * *Transformation*: $Z_1 = X W_1 + b_1$
  * *Activation*: Rectified Linear Unit ($\text{ReLU}(z) = \max(0, z)$), providing piecewise linearity while mitigating gradient vanishing.
* **Output Layer (`Z2`, `A2`)**: 10 units representing class categories $0$ through $9$.
  * *Transformation*: $Z_2 = A_1 W_2 + b_2$
  * *Activation*: Normalized Exponential ($\text{Softmax}$), mapping raw logit scores into an explicit categorical probability distribution where $\sum_{k=0}^{9} P(y=k \mid x) = 1.0$.

---

## Parameter Initialization Strategy
To prevent vanishing or exploding signal variance across network depths:
* **Hidden Weights ($W_1$)**: He (Kaiming) Normal initialization ($W \sim \mathcal{N}(0, \sigma^2)$ where $\sigma = \sqrt{2 / n_{\text{in}}}$), calibrated specifically for non-saturating ReLU activations.
* **Output Weights ($W_2$)**: Xavier (Glorot) Normal initialization ($\sigma = \sqrt{1 / n_{\text{in}}}$), matching the variance characteristics of linear logit projections.
* **Biases ($b_1, b_2$)**: Initialized strictly to zeros ($0.0$).

---

## Optimization & Training Pipeline
* **Algorithm**: Mini-batch Stochastic Gradient Descent (Mini-batch SGD).
* **Batch Size**: 64 to 128 samples per forward-backward step, striking a balance between vectorization throughput and gradient variance.
* **Epoch Shuffling**: Pseudo-random permutation of the training set at the onset of every epoch, ensuring mini-batches remain independent and identically distributed (i.i.d.) to prevent cyclical gradient oscillations.
* **Learning Rate ($\alpha$)**: Fixed step size ($0.10 \sim 0.15$), delivering monotonic loss decay without requiring complex momentum schedules.

---

## Loss Formulation & Numerical Stability
* **Loss Function**: Multi-class Categorical Cross-Entropy:
  $$J = -\frac{1}{m} \sum_{i=1}^{m} \sum_{k=0}^{9} Y_{i, k} \ln(\hat{Y}_{i, k} + \epsilon)$$
* **Log-Sum-Exp Trick**: The Softmax layer subtracts the row-wise maximum logit ($\mathbf{z} - \max(\mathbf{z})$) prior to exponentiation, preventing floating-point overflow (`inf` / `NaN`).
* **Zero Guard ($\epsilon$)**: A clipping factor ($\epsilon = 10^{-15}$) is added to predicted probabilities to safeguard against undefined logarithmic operations ($\ln(0)$).
* **Combined Gradient Simplification**: Analytically differentiating the combined Softmax and Cross-Entropy loss yields an exact, branch-free error term for the output layer:
  $$\frac{\partial J}{\partial Z_2} = \hat{Y} - Y_{\text{one\_hot}}$$

---

## Comprehensive Mathematical Pipeline & Tensor Shapes

The end-to-end forward-backward computational graph, annotated with precise matrix dimensionalities:

```text
====================================================================================================
STAGE                     OPERATION / FORMULA                                TENSOR DIMENSIONS
====================================================================================================
[ Input Mini-Batch ]      X                                                  (m, 784)
                          │
                          ▼
[ Layer 1 Linear ]        Z1 = X · W1 + b1                                   (m, 128)
                          │     ├── W1 ~ He Normal                           (784, 128)
                          │     └── b1 Broadcast Addition                    (1, 128)
                          ▼
[ Layer 1 Activation ]    A1 = ReLU(Z1) = max(0, Z1)                         (m, 128)
                          │
                          ▼
[ Layer 2 Linear ]        Z2 = A1 · W2 + b2                                  (m, 10)
                          │     ├── W2 ~ Xavier Normal                       (128, 10)
                          │     └── b2 Broadcast Addition                    (1, 10)
                          ▼
[ Layer 2 Activation ]    A2 = Softmax(Z2) = exp(Z2 - max(Z2)) / sum(...)    (m, 10)
                          │
                          ▼
[ Cross-Entropy Loss ]    J  = - (1/m) · Σ Σ [ Y_onehot ⊙ ln(A2 + 1e-15) ]    Scalar float
====================================================================================================
                                      BACKPROPAGATION ENGINE
====================================================================================================
[ Output Gradient ]       dZ2 = A2 - Y_onehot                                (m, 10)
                          │
                          ├──> dW2 = (1/m) · (A1)^T · dZ2                    (128, 10)
                          ├──> db2 = (1/m) · Σ_rows (dZ2)                    (1, 10)
                          │
                          ▼
[ Hidden Back-flow ]      dA1 = dZ2 · (W2)^T                                 (m, 128)
                          │
                          ▼
[ ReLU Gate Back-flow ]   dZ1 = dA1 ⊙ I(Z1 > 0)                              (m, 128)
                          │
                          ├──> dW1 = (1/m) · (X)^T · dZ1                     (784, 128)
                          └──> db1 = (1/m) · Σ_rows (dZ1)                    (1, 128)
====================================================================================================
                                   PARAMETER OPTIMIZATION (SGD)
====================================================================================================
[ Weights & Biases ]      W1 := W1 - α · dW1                                 (784, 128)
                          b1 := b1 - α · db1                                 (1, 128)
                          W2 := W2 - α · dW2                                 (128, 10)
                          b2 := b2 - α · db2                                 (1, 10)
====================================================================================================
```

## Detailed Repository Architecture
Plaintext
ai-labs/machine_learning/
├── number_identifier.py      # Core modular engine containing:
│                             #   - Mathematical primitives (relu, softmax, one_hot)
│                             #   - DigitIdentifier model class (init, forward, backward, train, predict)
│                             #   - Automated dataset acquisition pipeline via urllib
│                             #   - Custom image inference handler (PIL image pipeline)
├── mnist.npz                 # Binary compressed NumPy archive (60k train + 10k test arrays, ~11MB)
│                             #   - Downloaded once automatically; bypassed on subsequent runs
├── my_digit.png              # (Optional) User-supplied hand-drawn digit test image
└── README.md                 # Complete system design documentation and theoretical derivatio

## 🚀 Quickstart & Execution Guide

### 1. Environment Setup
The project relies solely on Python's standard library along with `numpy` for linear algebra and `pillow` for custom image preprocessing. Ensure Python 3.8 or higher is installed:

```bash
pip install numpy pillow
```

### 2. Run Training and Benchmark Evaluation
Launch the end-to-end training and testing routine via PowerShell or your terminal:

```bash
python number_identifier.py
```

**What happens automatically under the hood:**
1. **Zero-Setup Data Ingestion**: Checks the local project directory for `mnist.npz`. If absent, securely pulls the lightweight 11MB binary archive from Google Cloud Storage via Python's standard `urllib`.
2. **Tensor Preparation**: Unpacks 60,000 training and 10,000 test arrays, normalizes pixel intensities from [0, 255] into floating-point range [0.0, 1.0], and flattens them into 784-dimensional row vectors.
3. **Mini-Batch Optimization**: Runs 15 training epochs (batch size: 128, learning rate: 0.1) with full dataset reshuffling at each epoch.
4. **Validation**: Evaluates generalization performance on the untouched 10,000-sample test partition and prints side-by-side truth-versus-prediction arrays for inspection.

---

## 🖌️ Testing with Custom Hand-Drawn Images

The embedded inference pipeline supports reading and classifying arbitrary image files (e.g., drawn using MS Paint, macOS Preview, or a smartphone sketch tool):

### Drawing Guidelines
* **Brush Stroke**: Use a medium-to-thick solid stroke (thin 1-pixel digital pencils vanish when downsampled to 28x28).
* **Centering**: Position the digit near the canvas center, leaving approximately 15%–20% padding around the edges to match the MNIST structural distribution.
* **Format & Location**: Save your image as `my_digit.png` directly in the project root folder.

### Automated Image Transformation Pipeline
When `predict_custom_image("my_digit.png", model)` is invoked, the preprocessing pipeline applies the following sequential operations:
1. **Luma Extraction**: Strips alpha and RGB channels, converting the image into single-channel 8-bit grayscale (`L` mode).
2. **High-Fidelity Resampling**: Shrinks the source canvas to exactly 28x28 pixels using high-order Lanczos interpolation to preserve smooth, non-aliased stroke boundaries.
3. **Dynamic Contrast Inversion**: Evaluates border pixel intensities. If outer perimeter pixels average > 128 (standard black ink on white paper), the values are automatically inverted to black background with white stroke (0 -> background, 1 -> stroke).
4. **Logit Extraction & Probability Ranking**: Passes the (1, 784) vector through forward inference, extracting top-1 classification along with top-3 calibrated confidence percentages.

---

## 📊 Key Highlights & Engineering Choices

* **Analytic Backpropagation**: All gradient equations (dJ/dW, dJ/db, and inter-layer backflow dJ/dA_prev) are explicitly derived via multivariable chain rule—no automated autograd engines or black-box abstractions.
* **Controlled Variance Scaling**: Weights are initialized via He/Kaiming normal distributions (sigma = sqrt(2 / n_in)) for the ReLU hidden layer and Xavier/Glorot normal distributions (sigma = sqrt(1 / n_in)) for the Softmax output layer, keeping activation variance stable across layer depths.
* **Numerical Safeguards**: Softmax exponentiation incorporates max-logit subtraction (Z - max(Z)) to guarantee immunity to floating-point overflow (`inf`/`NaN`). Cross-entropy evaluates with an epsilon floor (1e-15) to prevent undefined zero logarithms (ln(0)).
* **Pure Vectorization**: All mini-batch steps, activation maps, and gradient contractions are expressed as vectorized NumPy matrix multiplications (`np.dot`), bypassing Python-level loops to maximize hardware cache locality and BLAS acceleration.

---

## 📝 License

Distributed under the **MIT License**. Free for educational, academic, and experimental use.