#include "DriftDiffusion.h"

// MOOSE includes
#include "MooseVariable.h"

registerMooseObject("ZapdosApp", DriftDiffusion);

template <>
InputParameters
validParams<DriftDiffusion>()
{
  InputParameters params = validParams<Kernel>();
  params.addCoupledVar(
      "potential", "The gradient of the potential will be used to compute the advection velocity.");
  params.addRequiredParam<Real>("position_units", "Units of position.");
  params.addParam<Real>("EField",
                        "Optionally can use a specified electric field for 1D "
                        "simulations in place of a potential variable");
  params.addParam<Real>("mu", "The mobility.");
  params.addParam<Real>("diff", "The diffusivity.");
  params.addParam<Real>("sign", "The sign of the charged particle.");
  params.addParam<bool>("use_material_props", true, "Whether to use a material for properties.");
  return params;
}

// This diffusion kernel should only be used with species whose values are in the logarithmic form.

DriftDiffusion::DriftDiffusion(const InputParameters & parameters)
  : Kernel(parameters),

    _r_units(1. / getParam<Real>("position_units")),

    _mu(getParam<bool>("use_material_props") ? getMaterialProperty<Real>("mu" + _var.name())
                                             : _user_mu),
    _sign(getParam<bool>("use_material_props") ? getMaterialProperty<Real>("sgn" + _var.name())
                                               : _user_sign),
    _diffusivity(getParam<bool>("use_material_props")
                     ? getMaterialProperty<Real>("diff" + _var.name())
                     : _user_diff),

    // Coupled variables
    _potential_id(coupled("potential")),
    _grad_potential(isCoupled("potential") ? coupledGradient("potential") : _minus_e_field)
{
  if (!(isCoupled("potential") || parameters.isParamSetByUser("EField")))
    mooseError("You must either couple in a potential variable or set an EField.");

  if (!(isCoupled("potential")))
    _minus_e_field.resize(_fe_problem.getMaxQps(), RealGradient(-getParam<Real>("EField")));
  auto max_qps = _fe_problem.getMaxQps();
  _user_diff.resize(max_qps);
  _user_mu.resize(max_qps);
  _user_sign.resize(max_qps);
  for (decltype(max_qps) qp = 0; qp < max_qps; ++qp)
  {
    _user_diff[qp] = getParam<Real>("diff");
    _user_mu[qp] = getParam<Real>("mu");
    _user_sign[qp] = getParam<Real>("sign");
  }
}

DriftDiffusion::~DriftDiffusion() {}

Real
DriftDiffusion::computeQpResidual()
{
  return _mu[_qp] * _sign[_qp] * std::exp(_u[_qp]) * -_grad_potential[_qp] * _r_units *
             -_grad_test[_i][_qp] * _r_units -
         _diffusivity[_qp] * std::exp(_u[_qp]) * _grad_u[_qp] * _r_units * -_grad_test[_i][_qp] *
             _r_units;
}

Real
DriftDiffusion::computeQpJacobian()
{
  return _mu[_qp] * _sign[_qp] * std::exp(_u[_qp]) * _phi[_j][_qp] * -_grad_potential[_qp] *
             _r_units * -_grad_test[_i][_qp] * _r_units -
         _diffusivity[_qp] *
             (std::exp(_u[_qp]) * _grad_phi[_j][_qp] * _r_units +
              std::exp(_u[_qp]) * _phi[_j][_qp] * _grad_u[_qp] * _r_units) *
             -_grad_test[_i][_qp] * _r_units;
}

Real
DriftDiffusion::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _potential_id)
  {
    return _mu[_qp] * _sign[_qp] * std::exp(_u[_qp]) * -_grad_phi[_j][_qp] * _r_units *
           -_grad_test[_i][_qp] * _r_units;
  }
  else
  {
    return 0.;
  }
}
