#include "PADMMLasso.h"
#include "DataStd.h"

#ifdef _OPENMP
#include <omp.h>
#endif

using Eigen::MatrixXd;
using Eigen::VectorXd;
using Eigen::ArrayXd;
using Eigen::ArrayXXd;
using Rcpp::as;
using Rcpp::List;
using Rcpp::IntegerVector;
using Rcpp::NumericVector;
using Rcpp::Named;

typedef Eigen::Map<MatrixXd> MapMat;
typedef Eigen::Map<VectorXd> MapVec;
typedef Eigen::Map<ArrayXd>  MapArray;

RcppExport SEXP admm_parlasso(SEXP x_, SEXP y_, SEXP lambda_,
                              SEXP nlambda_, SEXP lmin_ratio_,
                              SEXP standardize_, SEXP intercept_,
                              SEXP nthread_, SEXP opts_)
{
BEGIN_RCPP

    MatrixXd datX(as<MatrixXd>(x_));
    VectorXd datY(as<VectorXd>(y_));
    
    int n = datX.rows();
    int p = datX.cols();
    ArrayXd lambda(as<ArrayXd>(lambda_));
    int nlambda = lambda.size();
    
    List opts(opts_);
    int maxit = as<int>(opts["maxit"]);
    double eps_abs = as<double>(opts["eps_abs"]);
    double eps_rel = as<double>(opts["eps_rel"]);
    double rho_ratio = as<double>(opts["rho_ratio"]);

    bool standardize = as<bool>(standardize_);
    bool intercept = as<bool>(intercept_);
    DataStd datstd(n, p, standardize, intercept);
    datstd.standardize(datX, datY);

    int nthread = as<int>(nthread_);
#ifdef _OPENMP
    omp_set_num_threads(nthread);
#endif

    PADMMLasso_Master solver(datX, datY, nthread, eps_abs, eps_rel);
    if(nlambda < 1)
    {
        double lmax = solver.lambda_max() / n * datstd.get_scaleY();;
        double lmin = as<double>(lmin_ratio_) * lmax;
        lambda.setLinSpaced(as<int>(nlambda_), log(lmax), log(lmin));
        lambda = lambda.exp();
        nlambda = lambda.size();
    }

    ArrayXXd beta(p + 1, nlambda);
    IntegerVector niter(nlambda);
    double ilambda = 0.0;

    for(int i = 0; i < nlambda; i++)
    {
        ilambda = lambda[i] * n / datstd.get_scaleY();
        if(i == 0)
            solver.init(ilambda, rho_ratio * ilambda);
        else
            solver.init_warm(ilambda);

        niter[i] = solver.solve(maxit);
        beta.col(i).segment(1, p) = solver.get_z();
        datstd.recover(beta(0, i), beta.col(i).segment(1, p));
    }

    return List::create(Named("lambda") = lambda,
                        Named("beta") = beta,
                        Named("niter") = niter);

END_RCPP
}