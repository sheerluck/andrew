from forbiddenfruit import curse
from re import sub
import pyomo.environ as o

curse(str, "_", lambda txt: sub("\n[ ]*\|", "\n", txt))

model = o.ConcreteModel()
model.x = o.Var(domain=o.NonNegativeReals)
model.profit = o.Objective (expr = 40*model.x,
                            sense = o.maximize)
model.demand = o.Constraint(expr = model.x <= 40)
model.laborA = o.Constraint(expr = model.x <= 80)
model.laborB = o.Constraint(expr = 2*model.x <= 100)

for so in ["cbc",
           "glpk",
           "ipopt",
           "mindtpy",
           "mpec_minlp",
           "mpec_nlp",
           "multistart"]:
    o.SolverFactory(so).solve(model)
    print(f"""
      |{so}:
      |  x      = {round(model.x(),      3)},
      |  profit = {round(model.profit(), 3)}"""._())
