# Copyright 2022 Stefano Cottafavi <stefano.cottafavi@gmail.com>.
# SPDX-License-Identifier: GPL-2.0-only

import math
import numpy as np
import kimotor_linalg as kla
import wx

def parallel(r1,r2, dr,th,turns,dir):
        """ Compute layout points for coil with sides always aligned to radius

        Args:
            r1 (int): coil inner radius
            r2 (int): coil outer radius
            dr (int): spacing between coil loops (and also adj. coils)
            th (float): coil trapezoid angle 
            turns (int): number of coil loops (windings)
            dir (int): coil direction, from larger to smaller loop (0:CW normal, 1:CCW rverse)

        Returns:
            matrix, matrix, matrix: corners (excl. arc mids), outer arc mids, inner arc mids
        """

        pts = []
        mds = []
        mdsi = []

        # points 
        c = [0,0,0]
        l0 = np.array([ c, [r1*math.cos(th/2), r1*math.sin(th/2), 0] ])
        l0 = kla.line_offset(l0, -dr)
        
        for l in range(turns):
            # offset line
            lr = kla.line_offset(l0, -l*dr)
            # solve corners and order them
            pc1 = kla.circle_line_intersect(lr, c, r1+l*dr)
            pc1 = pc1[0:2]
            pc2 = kla.circle_line_intersect(lr, c, r2-l*dr)
            pc2 = pc2[0:2]
            pc3 = np.array([ pc2[0],-pc2[1] ])
            pc4 = np.array([ pc1[0],-pc1[1] ])
            if dir == 0:
                pts.extend([pc1, pc2, pc3, pc4])
            else:
                pts.extend([pc4, pc3, pc2, pc1])
            
            # solve outer and inner mid-points
            pm = np.array([ r2-l*dr, 0 ])
            pmi = np.array([ r1+l*dr, 0 ])
            mds.extend([pm])
            mdsi.extend([pmi])

        pm = np.matrix(pts)     # points, excl. arc mids
        mm = np.matrix(mds)     # arc (outer) mids only
        mmi = np.matrix(mdsi)    # arc (inner) mids only

        return pm, mm, mmi

def radial(r1,r2, dr,th,turns,dir):
        """ Compute layout points for coil with sides aligned to the local radial direction

        Args:
            r1 (int): coil inner radius
            r2 (int): coil outer radius
            dr (int): spacing between coil loops (and also adj. coils)
            th (float): coil trapezoid angle 
            turns (int): number of coil loops (windings)
            dir (int): coil direction, from larger to smaller loop (0:CW normal, 1:CCW rverse)

        Returns:
            matrix, matrix, matrix: corners (excl. arc mids), outer arc mids, inner arc mids
        """
 
        pts = []
        mds = []
        mdsi = []

        # center
        c = [0,0,0]
        
        # first line
        l0 = np.array([ c, [r1*math.cos(th/2), r1*math.sin(th/2), 0] ])

        for l in range(turns):
            # offset previous line
            lr = kla.line_offset(l0, -dr)
            
            # solve corners and order them
            pc1 = kla.circle_line_intersect(lr, c, r1+l*dr)
            pc1 = pc1[0:2]

            lr = [c, [pc1[0], pc1[1], 0]]

            pc2 = kla.circle_line_intersect(lr, c, r2-l*dr)
            pc2 = pc2[0:2]
            pc3 = np.array([ pc2[0],-pc2[1] ])
            pc4 = np.array([ pc1[0],-pc1[1] ])
            if dir == 0:
                pts.extend([pc1, pc2, pc3, pc4])
            else:
                pts.extend([pc4, pc3, pc2, pc1])
            
            # solve outer and inner mid-points
            pm = np.array([ r2-l*dr, 0 ])
            pmi = np.array([ r1+l*dr, 0 ])
            mds.extend([pm])
            mdsi.extend([pmi])

            #
            l0 = [ [pc1[0], pc1[1], 0], [pc2[0], pc2[1], 0] ] 

        pm = np.matrix(pts)     # points, excl. arc mids
        mm = np.matrix(mds)     # arc (outer) mids only
        mmi = np.matrix(mdsi)    # arc (inner) mids only

        return pm, mm, mmi
