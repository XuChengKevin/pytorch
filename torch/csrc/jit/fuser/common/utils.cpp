#include <torch/csrc/jit/fuser/common/utils.h>

#include <algorithm>
#include <iostream>

namespace torch {
namespace jit {
namespace fuser {

size_t getTensorRank(const std::shared_ptr<c10::TensorType>& tensor) {
  return *(tensor->dim());
}

size_t getNumNonCollapsibleDims(const std::shared_ptr<c10::TensorType>& tensor) {
  const c10::VaryingShape& sizes = tensor->sizes();
  const c10::VaryingStrides& strides = tensor->strides();

  const auto nDims = getTensorRank(tensor);

  if (nDims == 0) {
    return 0;
  }

  // Finds last dim with size > 1
  auto last = nDims - 1;
  for (int i = static_cast<int>(last); i >=0; --i) {
    const auto size = *(sizes[i]);
    if (size == 0 || size == 1) {
      continue;
    } else {
      last = i;
      break;
    }
  }

  size_t nNonCollapsibleDims = 1;
  auto collapse_value = *(strides[last]) * *(sizes[last]);
  for (int i = static_cast<int>(last - 1); i >= 0; --i) {
    const auto stride = *(strides[i]);
    const auto size = *(sizes[i]);

    // Sizes of zero or one are always collapsible
    if (size == 0 || size == 1) {
      continue;
    }

    if (stride != collapse_value) {
      ++nNonCollapsibleDims;
    }

    collapse_value = size * stride;
  }

  return nNonCollapsibleDims;
}

}}} // namespace torch::jit::fuser
