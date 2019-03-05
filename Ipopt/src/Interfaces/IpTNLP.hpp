// Copyright (C) 2004, 2009 International Business Machines and others.
// All Rights Reserved.
// This code is published under the Eclipse Public License.
//
// Authors:  Carl Laird, Andreas Waechter     IBM    2004-08-13

#ifndef __IPTNLP_HPP__
#define __IPTNLP_HPP__

#include "IpUtils.hpp"
#include "IpReferenced.hpp"
#include "IpException.hpp"
#include "IpAlgTypes.hpp"
#include "IpReturnCodes.hpp"

#include <map>

namespace Ipopt
{
// forward declarations
class IpoptData;
class IpoptCalculatedQuantities;
class IteratesVector;

/** Base class for all NLP's that use standard triplet matrix form and dense vectors.
 *
 *  This is the standard base class for all NLP's that use the standard triplet matrix
 *  form and dense vectors.
 *  The class TNLPAdapter then converts this interface to an interface that can be used
 *  directly by Ipopt.
 *
 *  This interface presents the problem form:
 *
 *     min f(x)
 *
 *     s.t. gL <= g(x) <= gU
 *
 *          xL <=  x   <= xU
 *
 *  In order to specify an equality constraint, set gL_i = gU_i = rhs.
 *  The value that indicates "infinity" for the bounds
 *  (i.e. the variable or constraint has no lower bound (-infinity) or upper bound (+infinity))
 *  is set through the option nlp_lower_bound_inf and nlp_upper_bound_inf, respectively.
 *  To indicate that a variable has no upper or lower bound, set the bound to
 *  -ipopt_inf or +ipopt_inf, respectively.
 */
class TNLP : public ReferencedObject
{
public:

   /** Linearity-types of variables and constraints. */
   enum LinearityType
   {
      LINEAR,    /**< Constraint/Variable is linear */
      NON_LINEAR /**< Constraint/Variable is non-linear */
   };

   /**@name Constructors/Destructors */
   //@{
   TNLP()
   { }

   /** Default destructor */
   virtual ~TNLP()
   { }
   //@}

   DECLARE_STD_EXCEPTION(INVALID_TNLP);

   /**@name methods to gather information about the NLP */
   //@{

   enum IndexStyleEnum
   {
      C_STYLE       = 0,
      FORTRAN_STYLE = 1
   };

   /** Method to request the initial information about the problem.
    *
    *  Ipopt uses this information when allocating the arrays
    *  that it will later ask you to fill with values. Be careful in this
    *  method since incorrect values will cause memory bugs which may be very
    *  difficult to find.
    *
    *  @param n           (out) Storage for the number of variables \f$x\f$
    *  @param m           (out) Storage for the number of constraints \f$g(x)\f$
    *  @param nnz_jac_g   (out) Storage for the number of nonzero entries in the Jacobian
    *  @param nnz_h_lag   (out) Storage for the number of nonzero entries in the Hessian
    *  @param index_style (out) Storage for the index style,
    *                     the numbering style used for row/col entries in the sparse matrix format
    *                     (TNLP::C_STYLE: 0-based, TNLP::FORTRAN_STYLE: 1-based; see also \ref TRIPLET)
    */
   // [TNLP_get_nlp_info]
   virtual bool get_nlp_info(
      Index&          n,
      Index&          m,
      Index&          nnz_jac_g,
      Index&          nnz_h_lag,
      IndexStyleEnum& index_style
   ) = 0;
   // [TNLP_get_nlp_info]

   typedef std::map<std::string, std::vector<std::string> > StringMetaDataMapType;
   typedef std::map<std::string, std::vector<Index> > IntegerMetaDataMapType;
   typedef std::map<std::string, std::vector<Number> > NumericMetaDataMapType;

   /** Method to request meta data for the variables and the constraints. */
   virtual bool get_var_con_metadata(
      Index                   n,
      StringMetaDataMapType&  var_string_md,
      IntegerMetaDataMapType& var_integer_md,
      NumericMetaDataMapType& var_numeric_md,
      Index                   m,
      StringMetaDataMapType&  con_string_md,
      IntegerMetaDataMapType& con_integer_md,
      NumericMetaDataMapType& con_numeric_md
   )
   {
      return false;
   }

   /** Method to request bounds on the variables and constraints.
    *
    *  @param n   (in) the number of variables \f$x\f$ in the problem
    *  @param x_l (out) the lower bounds \f$x^L\f$ for the variables \f$x\f$
    *  @param x_u (out) the upper bounds \f$x^U\f$ for the variables \f$x\f$
    *  @param m   (in) the number of constraints \f$g(x)\f$ in the problem
    *  @param g_l (out) the lower bounds \f$g^L\f$ for the constraints \f$g(x)\f$
    *  @param g_u (out) the upper bounds \f$g^U\f$ for the constraints \f$g(x)\f$
    *
    * The values of `n` and `m` that were specified in TNLP::get_nlp_info are passed
    * here for debug checking. Setting a lower bound to a value less than or
    * equal to the value of the option nlp_lower_bound_inf will cause \Ipopt to
    * assume no lower bound. Likewise, specifying the upper bound above or
    * equal to the value of the option nlp_upper_bound_inf will cause \Ipopt to
    * assume no upper bound. These options are set to -10<sup>19</sup> and
    * $10<sup>19</sup>, respectively, by default, but may be modified by changing the
    * options (see \ref OPTIONS).
    */
   // [TNLP_get_bounds_info]
   virtual bool get_bounds_info(
      Index   n,
      Number* x_l,
      Number* x_u,
      Index   m,
      Number* g_l,
      Number* g_u
   ) = 0;
   // [TNLP_get_bounds_info]

   /** Method to request scaling parameters.
    *
    *  This is only called if the options are set to retrieve user scaling.
    *  There, use_x_scaling (or use_g_scaling) should get set to true
    *  only if the variables (or constraints) are to be scaled.
    *  This method should return true only if the scaling parameters could
    *  be provided.
    */
   virtual bool get_scaling_parameters(
      Number& obj_scaling,
      bool&   use_x_scaling,
      Index   n,
      Number* x_scaling,
      bool&   use_g_scaling,
      Index   m,
      Number* g_scaling
   )
   {
      return false;
   }

   /** Method to request the variables linearity.
    *
    * The var_types array has been allocated with length at least n.
    * The default implementation just returns false and does not fill the array.
    */
   virtual bool get_variables_linearity(
      Index          n,
      LinearityType* var_types
   )
   {
      return false;
   }

   /** Method to request the constraints linearity.
    *
    *  The const_types array has been allocated with length at least m.
    *  The default implementation just returns false and does not fill the array.
    */
   virtual bool get_constraints_linearity(
      Index          m,
      LinearityType* const_types
   )
   {
      return false;
   }

   /** Method to request the starting point before iterating.
    *
    *  @param n      (in) the number of variables \f$x\f$ in the problem; it will have the same value that was specified in TNLP::get_nlp_info
    *  @param init_x (in) if true, this method must provide an initial value for \f$x\f$
    *  @param x      (out) the initial values for the primal variables \f$x\f$
    *  @param init_z (in) if true, this method must provide an initial value for the bound multipliers \f$z^L\f$ and \f$z^U\f$
    *  @param z_L    (out) the initial values for the bound multipliers \f$z^L\f$
    *  @param z_U    (out) the initial values for the bound multipliers \f$z^U\f$
    *  @param m      (in) the number of constraints \f$g(x)\f$ in the problem; it will have the same value that was specified in TNLP::get_nlp_info
    *  @param init_lambda (in) if true, this method must provide an initial value for the constraint multipliers \f$\lambda\f$
    *  @param lambda (out) the initial values for the constraint multipliers, \f$\lambda\f$
    *
    *  The boolean variables indicate whether the algorithm requires to
    *  have x, z_L/z_u, and lambda initialized, respectively.  If, for some
    *  reason, the algorithm requires initializations that cannot be
    *  provided, false should be returned and Ipopt will stop.
    *  The default options only require initial values for the primal
    *  variables \f$x\f$.
    *
    *  Note, that the initial values for bound multiplier components for
    *  absent bounds (\f$x^L_i=-\infty\f$ or \f$x^U_i=\infty\f$) are ignored.
    */
   // [TNLP_get_starting_point]
   virtual bool get_starting_point(
      Index   n,
      bool    init_x,
      Number* x,
      bool    init_z,
      Number* z_L,
      Number* z_U,
      Index   m,
      bool    init_lambda,
      Number* lambda
   ) = 0;
   // [TNLP_get_starting_point]

   /** Method to request an Ipopt warm start iterate.
    *
    *  Since this is only for expert users, a default dummy
    *  implementation is provided and returns false.
    */
   virtual bool get_warm_start_iterate(
      IteratesVector& warm_start_iterate /**< storage for warm start iterate in the form Ipopt requires it internally */
   )
   {
      return false;
   }

   /** Method to request the value of the objective function.
    *
    *  @param n     (in) the number of variables \f$x\f$ in the problem; it will have the same value that was specified in TNLP::get_nlp_info
    *  @param x     (in) the values for the primal variables \f$x\f$ at which the objective function \f$f(x)\f$ is to be evaluated
    *  @param new_x (in) false if any evaluation method (`eval_*`) was previously called with the same values in x, true otherwise.
    *                    This can be helpful when users have efficient implementations that calculate multiple outputs at once.
    *                    Ipopt internally caches results from the TNLP and generally, this flag can be ignored.
    *  @param obj_value (out) storage for the value of the objective function \f$f(x)\f$
    *
    * The variable n is passed in for convenience. It will have the same value that was specified in TNLP::get_nlp_info.
    */
   // [TNLP_eval_f]
   virtual bool eval_f(
      Index         n,
      const Number* x,
      bool          new_x,
      Number&       obj_value
   ) = 0;
   // [TNLP_eval_f]

   /** Method to request the gradient of the objective function.
    *
    *  @param n     (in) the number of variables \f$x\f$ in the problem; it will have the same value that was specified in TNLP::get_nlp_info
    *  @param x     (in) the values for the primal variables \f$x\f$ at which the gradient \f$\nabla f(x)\f$ is to be evaluated
    *  @param new_x (in) false if any evaluation method (`eval_*`) was previously called with the same values in x, true otherwise; see also TNLP::eval_f
    *  @param grad_f (out) array to store values of the gradient of the objective function \f$\nabla f(x)\f$.
    *                      The gradient array is in the same order as the \f$x\f$ variables
    *                      (i.e., the gradient of the objective with respect to `x[2]` should be put in `grad_f[2]`).
    */
   // [TNLP_eval_grad_f]
   virtual bool eval_grad_f(
      Index         n,
      const Number* x,
      bool          new_x,
      Number*       grad_f
   ) = 0;
   // [TNLP_eval_grad_f]

   /** Method to request the constraint values.
    *
    *  @param n     (in) the number of variables \f$x\f$ in the problem; it will have the same value that was specified in TNLP::get_nlp_info
    *  @param x     (in) the values for the primal variables \f$x\f$ at which the constraint functions \f$g(x)\f$ are to be evaluated
    *  @param new_x (in) false if any evaluation method (`eval_*`) was previously called with the same values in x, true otherwise; see also TNLP::eval_f
    *  @param m     (in) the number of constraints \f$g(x)\f$ in the problem; it will have the same value that was specified in TNLP::get_nlp_info
    *  @param g     (out) array to store constraint function values \f$g(x)\f$, do not add or subtract the bound values \f$g^L\f$ or \f$g^U\f$.
    */
   // [TNLP_eval_g]
   virtual bool eval_g(
      Index         n,
      const Number* x,
      bool          new_x,
      Index         m,
      Number*       g
   ) = 0;
   // [TNLP_eval_g]

   /** Method to request either the sparsity structure or the values of the Jacobian of the constraints.
    *
    * The Jacobian is the matrix of derivatives where the derivative of
    * constraint function \f$g_i\f$ with respect to variable \f$x_j\f$ is placed in row
    * \f$i\f$ and column \f$j\f$.
    * See \ref TRIPLET for a discussion of the sparse matrix format used in this method.
    *
    *  @param n     (in) the number of variables \f$x\f$ in the problem; it will have the same value that was specified in TNLP::get_nlp_info
    *  @param x     (in) first call: NULL; later calls: the values for the primal variables \f$x\f$ at which the constraint Jacobian \f$\nabla g(x)^T\f$ is to be evaluated
    *  @param new_x (in) false if any evaluation method (`eval_*`) was previously called with the same values in x, true otherwise; see also TNLP::eval_f
    *  @param m     (in) the number of constraints \f$g(x)\f$ in the problem; it will have the same value that was specified in TNLP::get_nlp_info
    *  @param nele_jac (in) the number of nonzero elements in the Jacobian; it will have the same value that was specified in TNLP::get_nlp_info
    *  @param iRow  (out) first call: array of length nele_jac to store the row indices of entries in the Jacobian of the constraints; later calls: NULL
    *  @param jCol  (out) first call: array of length nele_jac to store the column indices of entries in the Jacobian of the constraints; later calls: NULL
    *  @param values (out) first call: NULL; later calls: array of length nele_jac to store the values of the entries in the Jacobian of the constraints
    *
    * @note The arrays iRow and jCol only need to be filled once.
    * If the iRow and jCol arguments are not NULL (first call to this function),
    * then Ipopt expects that the sparsity structure of the Jacobian
    * (the row and column indices only) are written into iRow and jCol.
    * At this call, the arguments `x` and `values` will be NULL.
    * If the arguments `x` and `values` are not NULL, then Ipopt
    * expects that the value of the Jacobian as calculated from array `x`
    * is stored in array `values` (using the same order as used when
    * specifying the sparsity structure).
    * At this call, the arguments `iRow` and `jCol` will be NULL.
    */
   // [TNLP_eval_jac_g]
   virtual bool eval_jac_g(
      Index         n,
      const Number* x,
      bool          new_x,
      Index         m,
      Index         nele_jac,
      Index*        iRow,
      Index*        jCol,
      Number*       values
   ) = 0;
   // [TNLP_eval_jac_g]

   /** Method to request either the sparsity structure or the values of the Hessian of the Lagrangian.
    *
    * The Hessian matrix that Ipopt uses is
    * \f[ \sigma_f \nabla^2 f(x_k) + \sum_{i=1}^m\lambda_i\nabla^2 g_i(x_k) \f]
    * for the given values for \f$x\f$, \f$\sigma_f\f$, and \f$\lambda\f$.
    * See \ref TRIPLET for a discussion of the sparse matrix format used in this method.
    *
    *  @param n     (in) the number of variables \f$x\f$ in the problem; it will have the same value that was specified in TNLP::get_nlp_info
    *  @param x     (in) first call: NULL; later calls: the values for the primal variables \f$x\f$ at which the Hessian is to be evaluated
    *  @param new_x (in) false if any evaluation method (`eval_*`) was previously called with the same values in x, true otherwise; see also TNLP::eval_f
    *  @param obj_factor (in) factor \f$\sigma_f\f$ in front of the objective term in the Hessian
    *  @param m     (in) the number of constraints \f$g(x)\f$ in the problem; it will have the same value that was specified in TNLP::get_nlp_info
    *  @param lambda (in) the values for the constraint multipliers \f$\lambda\f$ at which the Hessian is to be evaluated
    *  @param new_lambda (in) false if any evaluation method was previously called with the same values in lambda, true otherwise
    *  @param nele_hess (in) the number of nonzero elements in the Hessian; it will have the same value that was specified in TNLP::get_nlp_info
    *  @param iRow  (out) first call: array of length nele_hess to store the row indices of entries in the Hessian; later calls: NULL
    *  @param jCol  (out) first call: array of length nele_hess to store the column indices of entries in the Hessian; later calls: NULL
    *  @param values (out) first call: NULL; later calls: array of length nele_hess to store the values of the entries in the Hessian
    *
    * @note The arrays iRow and jCol only need to be filled once.
    * If the iRow and jCol arguments are not NULL (first call to this function),
    * then Ipopt expects that the sparsity structure of the Hessian
    * (the row and column indices only) are written into iRow and jCol.
    * At this call, the arguments `x`, `lambda`, and `values` will be NULL.
    * If the arguments `x`, `lambda`, and `values` are not NULL, then Ipopt
    * expects that the value of the Hessian as calculated from arrays `x`
    * and `lambda` are stored in array `values` (using the same order as
    * used when specifying the sparsity structure).
    * At this call, the arguments `iRow` and `jCol` will be NULL.
    *
    * @attention As this matrix is symmetric, Ipopt expects that only the lower diagonal entries are specified.
    *
    * A default implementation is provided, in case the user
    * wants to set quasi-Newton approximations to estimate the second
    * derivatives and doesn't not need to implement this method.
    */
   // [TNLP_eval_h]
   virtual bool eval_h(
      Index         n,
      const Number* x,
      bool          new_x,
      Number        obj_factor,
      Index         m,
      const Number* lambda,
      bool          new_lambda,
      Index         nele_hess,
      Index*        iRow,
      Index*        jCol,
      Number*       values
   )
   // [TNLP_eval_h]
   {
      return false;
   }
   //@}

   /** @name Solution Methods */
   //@{
   /** This method is called when the algorithm has finished (successfully or not) so the TNLP can digest the outcome, e.g., store/write the solution, if any.
    *
    *  @param status @parblock (in) gives the status of the algorithm
    *   - SUCCESS: Algorithm terminated successfully at a locally optimal
    *     point, satisfying the convergence tolerances (can be specified
    *     by options).
    *   - MAXITER_EXCEEDED: Maximum number of iterations exceeded (can be specified by an option).
    *   - CPUTIME_EXCEEDED: Maximum number of CPU seconds exceeded (can be specified by an option).
    *   - STOP_AT_TINY_STEP: Algorithm proceeds with very little progress.
    *   - STOP_AT_ACCEPTABLE_POINT: Algorithm stopped at a point that was converged, not to "desired" tolerances, but to "acceptable" tolerances (see the acceptable-... options).
    *   - LOCAL_INFEASIBILITY: Algorithm converged to a point of local infeasibility. Problem may be infeasible.
    *   - USER_REQUESTED_STOP: The user call-back function TNLP::intermediate_callback returned false, i.e., the user code requested a premature termination of the optimization.
    *   - DIVERGING_ITERATES: It seems that the iterates diverge.
    *   - RESTORATION_FAILURE: Restoration phase failed, algorithm doesn't know how to proceed.
    *   - ERROR_IN_STEP_COMPUTATION: An unrecoverable error occurred while Ipopt tried to compute the search direction.
    *   - INVALID_NUMBER_DETECTED: Algorithm received an invalid number (such as NaN or Inf) from the NLP; see also option check_derivatives_for_nan_inf).
    *   - INTERNAL_ERROR: An unknown internal error occurred.
    *   @endparblock
    *  @param n     (in) the number of variables \f$x\f$ in the problem; it will have the same value that was specified in TNLP::get_nlp_info
    *  @param x     (in) the final values for the primal variables
    *  @param z_L   (in) the final values for the lower bound multipliers
    *  @param z_U   (in) the final values for the upper bound multipliers
    *  @param m     (in) the number of constraints \f$g(x)\f$ in the problem; it will have the same value that was specified in TNLP::get_nlp_info
    *  @param g     (in) the final values of the constraint functions
    *  @param lambda (in) the final values of the constraint multipliers
    *  @param obj_value (in) the final value of the objective function
    *  @param ip_data (in) provided for expert users
    *  @param ip_cq (in) provided for expert users
    */
   // [TNLP_finalize_solution]
   virtual void finalize_solution(
      SolverReturn               status,
      Index                      n,
      const Number*              x,
      const Number*              z_L,
      const Number*              z_U,
      Index                      m,
      const Number*              g,
      const Number*              lambda,
      Number                     obj_value,
      const IpoptData*           ip_data,
      IpoptCalculatedQuantities* ip_cq
   ) = 0;
   // [TNLP_finalize_solution]

   /** This method is called just before finalize_solution.  With
    *  this method, the algorithm returns any metadata collected
    *  during its run, including the metadata provided by the user
    *  with the above get_var_con_metadata.  Each metadata can be of
    *  type string, integer, and numeric. It can be associated to
    *  either the variables or the constraints.  The metadata that
    *  was associated with the primal variable vector is stored in
    *  var_..._md.  The metadata associated with the constraint
    *  multipliers is stored in con_..._md.  The metadata associated
    *  with the bound multipliers is stored in var_..._md, with the
    *  suffixes "_z_L", and "_z_U", denoting lower and upper
    *  bounds.
    */
   virtual void finalize_metadata(
      Index                         n,
      const StringMetaDataMapType&  var_string_md,
      const IntegerMetaDataMapType& var_integer_md,
      const NumericMetaDataMapType& var_numeric_md,
      Index                         m,
      const StringMetaDataMapType&  con_string_md,
      const IntegerMetaDataMapType& con_integer_md,
      const NumericMetaDataMapType& con_numeric_md
   )
   { }

   /** Intermediate Callback method for the user.
    *
    *  Providing dummy default implementation.
    *  For details see IntermediateCallBack in IpNLP.hpp.
    */
   virtual bool intermediate_callback(
      AlgorithmMode              mode,
      Index                      iter,
      Number                     obj_value,
      Number                     inf_pr,
      Number                     inf_du,
      Number                     mu,
      Number                     d_norm,
      Number                     regularization_size,
      Number                     alpha_du,
      Number                     alpha_pr,
      Index                      ls_trials,
      const IpoptData*           ip_data,
      IpoptCalculatedQuantities* ip_cq
   )
   {
      return true;
   }
   //@}

   /** @name Methods for quasi-Newton approximation.
    *
    *  If the second
    *  derivatives are approximated by Ipopt, it is better to do this
    *  only in the space of nonlinear variables.  The following
    *  methods are call by Ipopt if the quasi-Newton approximation is
    *  selected.  If -1 is returned as number of nonlinear variables,
    *  Ipopt assumes that all variables are nonlinear.  Otherwise, it
    *  calls get_list_of_nonlinear_variables with an array into which
    *  the indices of the nonlinear variables should be written - the
    *  array has the lengths num_nonlin_vars, which is identical with
    *  the return value of get_number_of_nonlinear_variables().  It
    *  is assumed that the indices are counted starting with 1 in the
    *  FORTRAN_STYLE, and 0 for the C_STYLE. */
   //@{
   virtual Index get_number_of_nonlinear_variables()
   {
      return -1;
   }

   virtual bool get_list_of_nonlinear_variables(
      Index  num_nonlin_vars,
      Index* pos_nonlin_vars
   )
   {
      return false;
   }
   //@}

private:
   /**@name Default Compiler Generated Methods
    * (Hidden to avoid implicit creation/calling).
    * These methods are not implemented and
    * we do not want the compiler to implement
    * them for us, so we declare them private
    * and do not define them. This ensures that
    * they will not be implicitly created/called.
    */
   //@{
   /** Copy Constructor */
   TNLP(
      const TNLP&
   );

   /** Default Assignment Operator */
   void operator=(
      const TNLP&
   );
   //@}
};

} // namespace Ipopt

#endif
