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

#include <Y/c++.h>
#include "calculator.h"

#include <iostream>
#include <sstream>
#include <cmath>

using namespace SigC;
using namespace std;

Calculator::Calculator (Y::Connection *y)
{
  window = new Y::Window (y, "Calculator");

  Y::GridLayout *grid = new Y::GridLayout (y);
  window -> setChild (grid);

  window -> requestClose.connect (bind (slot (&exit), EXIT_SUCCESS));

  result = new Y::Label (y, "0.");
  result->alignment.set("right");
  grid -> addWidget (result, 0, 0, 4, 1);

  Y::Button *button0 = new Y::Button (y, "0");
  button0 -> clicked.connect (bind(slot(*this, &Calculator::numberClicked), 0));
  grid -> addWidget (button0, 0, 5, 2, 1);

  Y::Button *button1 = new Y::Button (y, "1");
  button1 -> clicked.connect (bind(slot(*this, &Calculator::numberClicked), 1));
  grid -> addWidget (button1, 0, 4);

  Y::Button *button2 = new Y::Button (y, "2");
  button2 -> clicked.connect (bind(slot(*this, &Calculator::numberClicked), 2));
  grid -> addWidget (button2, 1, 4);

  Y::Button *button3 = new Y::Button (y, "3");
  button3 -> clicked.connect (bind(slot(*this, &Calculator::numberClicked), 3));
  grid -> addWidget (button3, 2, 4);

  Y::Button *button4 = new Y::Button (y, "4");
  button4 -> clicked.connect (bind(slot(*this, &Calculator::numberClicked), 4));
  grid -> addWidget (button4, 0, 3);

  Y::Button *button5 = new Y::Button (y, "5");
  button5 -> clicked.connect (bind(slot(*this, &Calculator::numberClicked), 5));
  grid -> addWidget (button5, 1, 3);

  Y::Button *button6 = new Y::Button (y, "6");
  button6 -> clicked.connect (bind(slot(*this, &Calculator::numberClicked), 6));
  grid -> addWidget (button6, 2, 3);

  Y::Button *button7 = new Y::Button (y, "7");
  button7 -> clicked.connect (bind(slot(*this, &Calculator::numberClicked), 7));
  grid -> addWidget (button7, 0, 2);

  Y::Button *button8 = new Y::Button (y, "8");
  button8 -> clicked.connect (bind(slot(*this, &Calculator::numberClicked), 8));
  grid -> addWidget (button8, 1, 2);

  Y::Button *button9 = new Y::Button (y, "9");
  button9 -> clicked.connect (bind(slot(*this, &Calculator::numberClicked), 9));
  grid -> addWidget (button9, 2, 2);

  Y::Button *buttonDot = new Y::Button (y, ".");
  buttonDot -> clicked.connect (slot(*this, &Calculator::pointClicked));
  grid -> addWidget (buttonDot, 2, 5);

  Y::Button *buttonEqu = new Y::Button (y, "=");
  buttonEqu -> clicked.connect (slot(*this, &Calculator::equalsClicked));
  grid -> addWidget (buttonEqu, 3, 4, 1, 2);

  Y::Button *buttonAdd = new Y::Button (y, "+");
  buttonAdd -> clicked.connect
       (bind(slot(*this, &Calculator::operatorClicked), PLUS));
  grid -> addWidget (buttonAdd, 3, 2, 1, 2);

  Y::Button *buttonSub = new Y::Button (y, "-");
  buttonSub -> clicked.connect
       (bind(slot(*this, &Calculator::operatorClicked), MINUS));
  grid -> addWidget (buttonSub, 3, 1);

  Y::Button *buttonMul = new Y::Button (y, "x");
  buttonMul -> clicked.connect
       (bind(slot(*this, &Calculator::operatorClicked), MULTIPLY));
  grid -> addWidget (buttonMul, 2, 1);

  Y::Button *buttonDiv = new Y::Button (y, "/");
  buttonDiv -> clicked.connect
       (bind(slot(*this, &Calculator::operatorClicked), DIVIDE));
  grid -> addWidget (buttonDiv, 1, 1);

  Y::Button *buttonClr = new Y::Button (y, "C");
  buttonClr -> clicked.connect (slot (*this, &Calculator::clearClicked));
  grid -> addWidget (buttonClr, 0, 1);

  setupOpTable();

  current = 0;
  decimalPlace = 0;

  window -> show ();
}

Calculator::~Calculator ()
{
}

/* setupOpTable should assign a precedence in opTable for each operator. */
void
Calculator::setupOpTable ()
{
  opTable.resize(num_ops);

  /* Add operators here and associate them with precedence */
  opTable[PLUS] = opTable[MINUS] = PREC_PLUSMINUS;
  opTable[MULTIPLY] = opTable[DIVIDE] = PREC_MULDIV;
  opTable[EQUALS] = PREC_EQUALS;
}

void
Calculator::setResult (double v)
{
  double integral;
  ostringstream s;
  s.precision (16);
  s << v;
  if (modf (v, &integral) < epsilon)
    s << '.';
  result->text.set(s.str());
}

void
Calculator::numberClicked (int n)
{
  if (decimalPlace > 0)
    {
      current += n / (pow (10.0, decimalPlace));
      ++decimalPlace;
    }
  else
    {
      current = (current * 10) + n;
    }
  setResult (current);
}

void
Calculator::pointClicked ()
{
  if (decimalPlace == 0)
    decimalPlace = 1;
  setResult (current);
}

void
Calculator::clearClicked ()
{
  current = 0;
  decimalPlace = 0;
  setResult (current);
}


/* operatorLessThan returns true if op1 should be evaluated before op2. */
bool
Calculator::operatorLessThan (Operator op1, Operator op2)
{
  return opTable[op1] < opTable[op2];
}

void
Calculator::evaluate (Operator max)
{
  while (!operators.empty() && operatorLessThan(operators.top(), max))
    {
       double operand1 = operands.top ();
       operands.pop ();
       double operand2 = operands.top ();
       operands.pop ();
       switch (operators.top ())
         {
         case PLUS:
           operands.push (operand2 + operand1);
           break;
         case MINUS:
           operands.push (operand2 - operand1);
           break;
         case MULTIPLY:
           operands.push (operand2 * operand1);
           break;
         case DIVIDE:
           operands.push (operand2 / operand1);
           break;
         case EQUALS:
           abort();
         }
       operators.pop ();
    }
}

void
Calculator::operatorClicked (Operator op)
{
  operands.push (current);
  evaluate (op);
  setResult (operands.top ());
  operators.push (op);

  current = 0;
  decimalPlace = 0;
}

void
Calculator::equalsClicked ()
{
  operands.push (current);
  evaluate (EQUALS);
  setResult (operands.top ());
  operands.pop ();

  current = 0;
  decimalPlace = 0;
}

int
main (int argc, char **argv)
{
  Y::Connection y;
  Calculator c(&y);
  y.run();
}

/* arch-tag: 4f1e6992-37c8-438d-b8bf-611aa3bc1ce3
 */
