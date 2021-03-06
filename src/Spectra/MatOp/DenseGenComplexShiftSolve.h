// Copyright (C) 2016 Yixuan Qiu <yixuan.qiu@cos.name>
//
// This Source Code Form is subject to the terms of the Mozilla
// Public License v. 2.0. If a copy of the MPL was not distributed
// with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef DENSE_GEN_COMPLEX_SHIFT_SOLVE_H
#define DENSE_GEN_COMPLEX_SHIFT_SOLVE_H

#include <Eigen/Core>
#include <Eigen/LU>
#include <stdexcept>

namespace Spectra {


///
/// \ingroup MatOp
///
/// This class defines the complex shift-solve operation on a general real matrix \f$A\f$,
/// i.e., calculating \f$y=\mathrm{Re}\{(A-\sigma I)^{-1}x\}\f$ for any complex-valued
/// \f$\sigma\f$ and real-valued vector \f$x\f$. It is mainly used in the
/// GenEigsComplexShiftSolver eigen solver.
///
template <typename Scalar>
class DenseGenComplexShiftSolve
{
private:
    typedef Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic> Matrix;
    typedef Eigen::Map<const Matrix> MapMat;
    typedef Eigen::Map< Eigen::Matrix<Scalar, Eigen::Dynamic, 1> > MapVec;

    typedef std::complex<Scalar> Complex;
    typedef Eigen::Matrix<Complex, Eigen::Dynamic, Eigen::Dynamic> ComplexMatrix;
    typedef Eigen::Matrix<Complex, Eigen::Dynamic, 1> ComplexVector;
    typedef Eigen::PartialPivLU<ComplexMatrix> ComplexSolver;

    typedef const Eigen::Ref<const Matrix> ConstGenericMatrix;

    const MapMat m_mat;
    const int m_n;
    ComplexSolver m_solver;
    ComplexVector m_x_cache;

public:
    ///
    /// Constructor to create the matrix operation object.
    ///
    /// \param mat_ An **Eigen** matrix object, whose type can be
    /// `Eigen::Matrix<Scalar, ...>` (e.g. `Eigen::MatrixXd` and
    /// `Eigen::MatrixXf`), or its mapped version
    /// (e.g. `Eigen::Map<Eigen::MatrixXd>`).
    ///
    DenseGenComplexShiftSolve(ConstGenericMatrix &mat_) :
        m_mat(mat_.data(), mat_.rows(), mat_.cols()),
        m_n(mat_.rows())
    {
        if(mat_.rows() != mat_.cols())
            throw std::invalid_argument("DenseGenComplexShiftSolve: matrix must be square");
    }

    ///
    /// Return the number of rows of the underlying matrix.
    ///
    int rows() { return m_n; }
    ///
    /// Return the number of columns of the underlying matrix.
    ///
    int cols() { return m_n; }

    ///
    /// Set the complex shift \f$\sigma\f$.
    ///
    /// \param sigmar Real part of \f$\sigma\f$.
    /// \param sigmai Imaginary part of \f$\sigma\f$.
    ///
    void set_shift(Scalar sigmar, Scalar sigmai)
    {
        ComplexMatrix cmat = m_mat.template cast<Complex>();
        cmat.diagonal().array() -= Complex(sigmar, sigmai);
        m_solver.compute(cmat);
        m_x_cache.resize(m_n);
        m_x_cache.setZero();
    }

    ///
    /// Perform the complex shift-solve operation
    /// \f$y=\mathrm{Re}\{(A-\sigma I)^{-1}x\}\f$.
    ///
    /// \param x_in  Pointer to the \f$x\f$ vector.
    /// \param y_out Pointer to the \f$y\f$ vector.
    ///
    // y_out = Re( inv(A - sigma * I) * x_in )
    void perform_op(Scalar *x_in, Scalar *y_out)
    {
        m_x_cache.real() = MapVec(x_in, m_n);
        MapVec y(y_out, m_n);
        y.noalias() = m_solver.solve(m_x_cache).real();
    }
};


} // namespace Spectra

#endif // DENSE_GEN_COMPLEX_SHIFT_SOLVE_H
