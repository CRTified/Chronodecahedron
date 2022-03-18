
phi = golden_ratio
deg2rad = lambda x: x * pi / 180

# Normal vector of faces are the vertices of an icosahedron (dual to dodecahedron)
faces = map(lambda x: vector(RR, x),
            [ (   0,    1,  phi) # ok 1
            , (  -1,  phi,    0) # ok 2
            , (   1,  phi,    0) # ok 3
            , ( phi,    0,    1) # ok 4
            , (   0,   -1,  phi) # ok 5
            , (-phi,    0,    1) # ok 6
            , (-phi,    0,   -1) # ok 7
            , (   0,    1, -phi) # ok 8
            , ( phi,    0,   -1) # ok 9
            , (   1, -phi,    0) # ok10
            , (  -1, -phi,    0) # ok11
            , (   0,   -1, -phi) # ok12
            ])

# Rotate along the x-axis. Formula based on sum of angles in a triangle and
# angle between face and edge in an icosahedron:
# Consider https://commons.wikimedia.org/wiki/File:Ikosaeder-gr.svg
# The desired rotation angle to rotate one vertex on the z axis is the
# angle on the origin, of the triangle between the origin, r_u and the extension
# of the line to P3.
angle = pi / 2 - (pi / 2 + atan((3 - sqrt(5)) / 2)) / 2
rotation_x = matrix(RR,
                [ [ 1, 0, 0]
                , [ 0, cos(angle), -sin(angle)]
                , [ 0, sin(angle),  cos(angle)]
                ])

print("      // Remember: Rotation might be numerically imprecise")
for i, f in enumerate(faces):
    res = (rotation_x * f).normalized()
    print("      /* {:2} */ {{ {:+2.7f}, {:+2.7f}, {:+2.7f} }},".format(i + 1, *res))
