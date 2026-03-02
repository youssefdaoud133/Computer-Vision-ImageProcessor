Based on the image provided, Histogram Equalization is a technique used to improve the contrast of an image by stretching out the intensity levels. Essentially, it takes an image where pixels are "bunched up" in a narrow range and spreads them across the full available spectrum (from $0$ to $L-1$).

Here is the step-by-step breakdown of how to perform the equalization using the formula shown in your slide:

---

### 1. Identify the Variables

First, you need to collect the data from your original image:

- **$L$**: The total number of possible grey levels (e.g., for an 8-bit image, $L = 256$; for the 3-bit example in the notes, $L = 8$).
- **$n$**: The total number of pixels in the image (Width × Height).
- **$n_i$**: The number of pixels that have the specific intensity value $i$.

### 2. Calculate the Cumulative Distribution Function (CDF)

The numerator in your formula, $(n_0 + n_1 + \dots + n_i)$, is the **cumulative sum** of pixels up to level $i$.

- For level 0, it's just $n_0$.
- For level 1, it's $n_0 + n_1$.
- For level 2, it's $n_0 + n_1 + n_2$, and so on.

### 3. Apply the Transformation Formula

To find the new intensity value for any pixel that originally had value $i$, apply the formula:

$$New\ Value = \left( \frac{\sum_{j=0}^{i} n_j}{n} \right) \times (L - 1)$$

**Breaking down the math:**

1. **Divide by $n$**: This calculates the probability/proportional rank of that intensity (normalizing the value between 0 and 1).
2. **Multiply by $(L - 1)$**: This scales that probability back to the actual range of your image (e.g., if your max level is 7, it scales it to a value between 0 and 7).
3. **Round the result**: Since pixel values must be integers, you round the final result to the nearest whole number.

---

### Summary of the Process

| Step          | Action                                          | Result                          |
| ------------- | ----------------------------------------------- | ------------------------------- |
| **Histogram** | Count how many pixels exist for each level $i$. | $n_i$                           |
| **PDF**       | Divide $n_i$ by total pixels $n$.               | Probability of level $i$        |
| **CDF**       | Add up the probabilities cumulatively.          | Cumulative probability (0 to 1) |
| **Map**       | Multiply CDF by $(L-1)$ and round.              | **The New Equalized Intensity** |
