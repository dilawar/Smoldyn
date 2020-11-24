# -*- coding: utf-8 -*-

__author__ = "Dilawar Singh"
__copyright__ = "Copyright 2020-, Dilawar Singh"
__maintainer__ = "Dilawar Singh"
__email__ = "dilawars@ncbs.res.in"

from dataclasses import dataclass
from typing import List
import smoldyn.types as T
from smoldyn import _smoldyn
from smoldyn._smoldyn import RevParam
from smoldyn.config import __logger__



class Species:
    """Chemical species.
    """

    def __init__(
        self,
        name: str,
        state: str = "soln",
        color = "",
        difc: float = 0.0,
        display_size: int = 2,
        mol_list: str = "",
        **kwargs
    ):
        """
        Parameters
        ----------
        name : str
            name of the species.
        state : str
            state of the species. One of the following: 
                soln, front, back, up, down, bsoln, all, none, some
        color : str or tuple of float (r, g, b), optional
            color of the species (default 'black')
        difc : float, optional
            diffusion coefficient (default 0.0, unit: μM^2/s)
        display_size : int, optional
            display size of the molecule (default 3px).
        mol_list : str, optional
            molecule list (default '')
        """
        self.name: str = name
        # assert self.name

        k = _smoldyn.addSpecies(self.name)
        assert k == _smoldyn.ErrorCode.ok, f"Failed to add molecule: {k}"

        if state not in _smoldyn.MolecState.__members__:
            raise NameError(
                f"{state} is not a valid MolecState. Available "
                "states are:{', '.join(_smoldyn.MolecState.__members__.keys())}"
            )

        self.state = _smoldyn.MolecState.__members__[state]

        self._difc: float = difc
        self._color : T.Color = color
        self._size: float = display_size

        self.difc: float = self._difc
        self.size: float = self._size
        self.color = self._color

        self._mol_list: str = mol_list
        if mol_list:
            self.mol_list: str = mol_list

    def __repr__(self):
        return f"<Molecule: {self.name}, difc={self.difc}, state={self.state}>"

    def setStyle(self):
        k = _smoldyn.setMoleculeStyle(self.name, self.state, self.size, self.color)
        assert k == _smoldyn.ErrorCode.ok, f"Failed to set style on {self}, {k}"

    @property
    def difc(self) -> float:
        return self._difc

    @difc.setter
    def difc(self, difc: float):
        assert self.state, "Please set 'state' first"
        self._difc = difc
        k = _smoldyn.setSpeciesMobility(self.name, self.state, self.difc)
        assert k == _smoldyn.ErrorCode.ok, f"Failed to set mobility: {k}"

    @property
    def mol_list(self) -> str:
        return self._mol_list

    @mol_list.setter
    def mol_list(self, val):
        k = _smoldyn.addMolList(val)
        assert k == _smoldyn.ErrorCode.ok, f"Failed to add mollist: {k}"
        k = _smoldyn.setMolList(self.name, self.state, val)
        assert (
            k == _smoldyn.ErrorCode.ok
        ), f"Failed to set mol_list={val} on {self}: {k}"
        self._mol_list = val

    @property
    def color(self) -> T.Color:
        return self._color

    @color.setter
    def color(self, color: str):
        self._color = color
        self.setStyle()

    @property
    def size(self):
        return self._size

    @size.setter
    def size(self, size: float):
        self._size = size
        self.setStyle()

    def addToSolution(
        self, mol: float, highpos: List[float] = [], lowpos: List[float] = []
    ):
        assert self.state == _smoldyn.MolecState.soln, (
            f"You can't use this function on a Species with type {self.state}"
        )
        k = _smoldyn.addSolutionMolecules(self.name, mol, lowpos, highpos)
        assert k == _smoldyn.ErrorCode.ok, f"Failed to add to solution: {k}"

class NullSpecies(Species):

    def __init__(self):
        self.name = ""
        self.state = _smoldyn.MolecState.__members__["all"]
        # No need to call super().__init__()



class Reaction:
    def __init__(self, name: str, subs: List[Species], prds: List[Species], rate: float=0):
        assert len(subs) < 3, "At most two reactants are supported"
        r1 = subs[0]
        r2 = subs[1] if len(subs) == 2 else Species("", "all")
        if not name:
            name = "r%d" % id(self)
        assert name
        self.name = name
        k = _smoldyn.addReaction(
            name,
            r1.name,
            r1.state,
            r2.name,
            r2.state,
            [x.name for x in prds],
            [x.state for x in prds],
            rate,
        )
        if k != _smoldyn.ErrorCode.ok:
            __logger__.warning(f" Reactant1 : {r1}")
            __logger__.warning(f" Reactant2 : {r2}")
            __logger__.warning(f" Subtrate  : {subs}")
            __logger__.warning(f" Products  : {prds}")
            raise RuntimeError(f"Failed to add reaction: {k}")

    def productPlacement(self, method: str, parameter: float, product: Species=NullSpecies(), position: List[float]=[]):
        assert method in ["none","irrev","confspread","bounce","pgem","pgemmax","pgemmaxw","ratio","unbindrad","pgem2","pgemmax2","ratio2","offset","fixed"]
        print("***********",self.name)
        k = _smoldyn.setReactionProducts(
            self.name,
            RevParam.__members__[method],
            parameter,
            product.name,
            position)
        assert k== _smoldyn.ErrorCode.ok, f"Failed to achieve productPlacement: {k}"

