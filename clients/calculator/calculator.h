/************************************************************************
 *   Copyright (C) Mark Thomas <markbt@efaref.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#ifndef YCALCULATOR_CALCULATOR_H
#define YCALCULATOR_CALCULATOR_H

#include <Y/c++.h>
#include <sigc++/sigc++.h>

#include <stack>

using namespace SigC;
using namespace std;

class Calculator : public SigC::Object
{
 private:
  Y::Window *window;
  Y::Label *result;

  enum Operator
    {
      PLUS, MINUS, DIVIDE, MULTIPLY, EQUALS
    };
  static const int num_ops = 5;

  /* this enum should be in order of actual precedence */
  enum OperatorPrecedence
    {
      PREC_MULDIV, PREC_PLUSMINUS, PREC_EQUALS
    };

  stack<double> operands;
  stack<Operator> operators;

  /* precedence list of operators in the enum */
  vector<OperatorPrecedence> opTable;

  double current;
  int decimalPlace;

  void setResult (double v);
  bool operatorLessThan (Operator, Operator);
  void evaluate (Operator max);

  static const double epsilon = 0.00000001;

  void setupOpTable ();

 public:
  Calculator (Y::Connection *y);
  virtual ~Calculator ();

  void numberClicked (int n);
  void pointClicked ();
  void clearClicked ();
  void operatorClicked (Operator op);
  void equalsClicked ();
};

#endif

/* arch-tag: 0062c097-9f61-4126-8147-8701cd192e10
 */
