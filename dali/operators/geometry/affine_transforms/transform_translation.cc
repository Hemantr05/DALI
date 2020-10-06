// Copyright (c) 2020, NVIDIA CORPORATION. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dali/operators/geometry/affine_transforms/transform_base_op.h"
#include "dali/pipeline/data/views.h"

namespace dali {

DALI_SCHEMA(TransformTranslation)
  .DocStr(R"code(Produces a translation affine transform matrix.

If another transform matrix is passed as an input, the operator applies translation to the matrix provided.

.. note::
    The output of this operator can be fed directly to the ``MT`` argument of ``CoordTransform`` operator.
)code")
  .AddArg(
    "offset",
    R"code(The translation vector.

The number of dimensions of the transform is inferred from this argument.)code",
    DALI_FLOAT_VEC, true)
  .NumInput(0, 1)
  .NumOutput(1)
  .AddParent("TransformAttr");

/**
 * @brief Translation transformation.
 */
class TransformTranslationCPU
    : public TransformBaseOp<CPUBackend, TransformTranslationCPU> {
 public:
  using SupportedDims = dims<1, 2, 3, 4, 5, 6>;

  explicit TransformTranslationCPU(const OpSpec &spec) :
      TransformBaseOp<CPUBackend, TransformTranslationCPU>(spec),
      offset_("offset", spec) {}

  template <typename T, int mat_dim>
  void DefineTransforms(span<affine_mat_t<T, mat_dim>> matrices) {
    constexpr int ndim = mat_dim - 1;
    assert(matrices.size() == static_cast<int>(offset_.size()));
    for (int i = 0; i < matrices.size(); i++) {
      auto &mat = matrices[i];
      auto *offset = offset_[i].data();
      mat = affine_mat_t<T, mat_dim>::identity();
      for (int d = 0; d < ndim; d++) {
        mat(d, ndim) = offset[d];
      }
    }
  }

  void ProcessArgs(const OpSpec &spec, const workspace_t<CPUBackend> &ws) {
    assert(offset_.IsDefined());
    offset_.Read(spec, ws);
    ndim_ = offset_[0].size();
  }

  bool IsConstantTransform() const {
    return !offset_.IsArgInput();
  }

 private:
  Argument<std::vector<float>> offset_;
};

DALI_REGISTER_OPERATOR(TransformTranslation, TransformTranslationCPU, CPU);

}  // namespace dali