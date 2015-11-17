/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ElectronEnergyLossFromElastic.h"


template<>
InputParameters validParams<ElectronEnergyLossFromElastic>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredCoupledVar("potential","The potential.");
  params.addRequiredCoupledVar("em", "The electron density.");
  return params;
}


ElectronEnergyLossFromElastic::ElectronEnergyLossFromElastic(const InputParameters & parameters) :
  Kernel(parameters),
    
  _diffem(getMaterialProperty<Real>("diffem")),
  _muem(getMaterialProperty<Real>("muem")),
  _alpha_iz(getMaterialProperty<Real>("alpha_iz")),
  _d_iz_d_actual_mean_en(getMaterialProperty<Real>("d_iz_d_actual_mean_en")),
  _d_muem_d_actual_mean_en(getMaterialProperty<Real>("d_muem_d_actual_mean_en")),
  _d_diffem_d_actual_mean_en(getMaterialProperty<Real>("d_diffem_d_actual_mean_en")),
  _mem(getMaterialProperty<Real>("mem")),
  _mGas(getMaterialProperty<Real>("mGas")),
  _alpha_el(getMaterialProperty<Real>("alpha_el")),
  _d_el_d_actual_mean_en(getMaterialProperty<Real>("d_el_d_actual_mean_en")),

  _grad_potential(coupledGradient("potential")),
  _em(coupledValue("em")),
  _grad_em(coupledGradient("em")),
  _potential_id(coupled("potential")),
  _em_id(coupled("em"))
{
}

ElectronEnergyLossFromElastic::~ElectronEnergyLossFromElastic()
{
}

Real
ElectronEnergyLossFromElastic::computeQpResidual()
{
  Real electron_flux_mag = (-_muem[_qp]*-_grad_potential[_qp]*std::exp(_em[_qp])-_diffem[_qp]*std::exp(_em[_qp])*_grad_em[_qp]).size();
  Real Eel = -3.0*_mem[_qp]/_mGas[_qp]*2.0/3*std::exp(_u[_qp]-_em[_qp]);
  Real el_term = _alpha_el[_qp] * electron_flux_mag * Eel;

  return -_test[_i][_qp]*el_term;
}

Real
ElectronEnergyLossFromElastic::computeQpJacobian()
{
  Real actual_mean_en = std::exp(_u[_qp]-_em[_qp]);
  Real d_actual_mean_en_d_mean_en = std::exp(_u[_qp]-_em[_qp])*_phi[_j][_qp];
  Real d_el_d_mean_en = _d_el_d_actual_mean_en[_qp] * d_actual_mean_en_d_mean_en;
  Real d_muem_d_mean_en = _d_muem_d_actual_mean_en[_qp] * d_actual_mean_en_d_mean_en;
  Real d_diffem_d_mean_en = _d_diffem_d_actual_mean_en[_qp] * d_actual_mean_en_d_mean_en;

  RealVectorValue electron_flux = -_muem[_qp]*-_grad_potential[_qp]*std::exp(_em[_qp])-_diffem[_qp]*std::exp(_em[_qp])*_grad_em[_qp];
  RealVectorValue d_electron_flux_d_mean_en = -d_muem_d_mean_en*-_grad_potential[_qp]*std::exp(_em[_qp])-d_diffem_d_mean_en*std::exp(_em[_qp])*_grad_em[_qp];
  Real electron_flux_mag = electron_flux.size();
  Real d_electron_flux_mag_d_mean_en = electron_flux*d_electron_flux_d_mean_en/(electron_flux_mag+std::numeric_limits<double>::epsilon());

  Real Eel = -3.0*_mem[_qp]/_mGas[_qp]*2.0/3*std::exp(_u[_qp]-_em[_qp]);
  Real d_Eel_d_mean_en = -3.0*_mem[_qp]/_mGas[_qp]*2.0/3*std::exp(_u[_qp]-_em[_qp])*_phi[_j][_qp];
  Real d_el_term_d_mean_en = (electron_flux_mag * d_el_d_mean_en + _alpha_el[_qp] * d_electron_flux_mag_d_mean_en)*Eel + electron_flux_mag*_alpha_el[_qp]*d_Eel_d_mean_en;

  return -_test[_i][_qp] * d_el_term_d_mean_en;
}

Real
ElectronEnergyLossFromElastic::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real actual_mean_en = std::exp(_u[_qp]-_em[_qp]);
  Real d_actual_mean_en_d_em = -std::exp(_u[_qp]-_em[_qp])*_phi[_j][_qp];
  Real d_el_d_em = _d_el_d_actual_mean_en[_qp] * d_actual_mean_en_d_em;
  Real d_muem_d_em = _d_muem_d_actual_mean_en[_qp] * d_actual_mean_en_d_em;
  Real d_diffem_d_em = _d_diffem_d_actual_mean_en[_qp] * d_actual_mean_en_d_em;

  RealVectorValue electron_flux = -_muem[_qp]*-_grad_potential[_qp]*std::exp(_em[_qp])-_diffem[_qp]*std::exp(_em[_qp])*_grad_em[_qp];
  RealVectorValue d_electron_flux_d_potential = -_muem[_qp]*-_grad_phi[_j][_qp]*std::exp(_em[_qp]);
  RealVectorValue d_electron_flux_d_em = -d_muem_d_em*-_grad_potential[_qp]*std::exp(_em[_qp])-_muem[_qp]*-_grad_potential[_qp]*std::exp(_em[_qp])*_phi[_j][_qp]-d_diffem_d_em*std::exp(_em[_qp])*_grad_em[_qp]-_diffem[_qp]*std::exp(_em[_qp])*_phi[_j][_qp]*_grad_em[_qp]-_diffem[_qp]*std::exp(_em[_qp])*_grad_phi[_j][_qp];
  Real electron_flux_mag = electron_flux.size();
  Real d_electron_flux_mag_d_potential = electron_flux*d_electron_flux_d_potential/(electron_flux_mag+std::numeric_limits<double>::epsilon());
  Real d_electron_flux_mag_d_em = electron_flux*d_electron_flux_d_em/(electron_flux_mag+std::numeric_limits<double>::epsilon());

  Real Eel = -3.0*_mem[_qp]/_mGas[_qp]*2.0/3*std::exp(_u[_qp]-_em[_qp]);
  Real d_Eel_d_em = -3.0*_mem[_qp]/_mGas[_qp]*2.0/3*std::exp(_u[_qp]-_em[_qp])*-_phi[_j][_qp];
  Real d_Eel_d_potential = 0.0;
  Real d_el_term_d_em = (electron_flux_mag * d_el_d_em + _alpha_el[_qp] * d_electron_flux_mag_d_em)*Eel + electron_flux_mag*_alpha_el[_qp]*d_Eel_d_em;
  Real d_el_term_d_potential = (_alpha_el[_qp] * d_electron_flux_mag_d_potential)*Eel + electron_flux_mag*_alpha_el[_qp]*d_Eel_d_potential;


  if (jvar == _potential_id)
    return -_test[_i][_qp] * d_el_term_d_potential;

  else if (jvar == _em_id)
    return -_test[_i][_qp] * d_el_term_d_potential;
  
  else
    return 0.0;
}