#ifndef TM0CYLINDRICAL_H
#define TM0CYLINDRICAL_H

#include "Kernel.h"

class TM0Cylindrical;

template<>
InputParameters validParams<TM0Cylindrical>();

// This diffusion kernel should only be used with species whose values are in the logarithmic form.

class TM0Cylindrical : public Kernel
{
public:
  TM0Cylindrical(const InputParameters & parameters);
  virtual ~TM0Cylindrical();

protected:

  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /* Real _r_units; */
  Real _omega;
  Real _eps_r;
  Real _mu0;
  Real _eps0;
};


#endif /* TM0CYLINDRICAL_H */
