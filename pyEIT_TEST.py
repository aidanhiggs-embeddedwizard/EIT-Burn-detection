import numpy as np
import matplotlib.pyplot as plt
import pyeit.mesh as mesh
import pyeit.eit.protocol as protocol
import pyeit.eit.greit as greit
import pyeit.eit.jac as jac
from pyeit.eit.interp2d import sim2pts

def plot_voltages(v_hom, v_inhom):
    fig_dataplot, (ax1, ax2) = plt.subplots(2, 1, figsize=(8, 6))
    ax1.plot(v_hom, 'b-')
    ax1.set_title('Homogeneous data')
    #ax1.set_xlabel('measurement index')
    ax1.set_ylabel('boundary voltage (V)')
    ax1.grid(True)

    ax2.plot(v_inhom, 'r-')
    ax2.set_title('inHomogeneous data')
    ax2.set_xlabel('measurement index')
    ax2.set_ylabel('boundary voltage (V)')
    ax2.grid(True)
    plt.show()

#plt.show()

def my_greit_solver(v_inhom, v_hom ,mesh_input, protocol_input):
    """
        Reconstruct using the GREIT algorithm
        plot reconstructions for different p-values and lambda values
    """
    p_values = [0.05, 0.15, 0.30]
    lamb_values = [0.001, 0.01, 0.1, 1.0]

    fig, axes = plt.subplots(len(p_values), len(lamb_values), constrained_layout=True, figsize=(12, 9))
    fig.suptitle('GREIT Solver')
    for row_index, p_val in enumerate(p_values):
        for col_index, lamb_val in enumerate(lamb_values):
            eit = greit.GREIT(mesh_input, protocol_input)
            eit.setup(p=p_val, lamb=lamb_val,perm=1,jac_normalized=True)

            ds = eit.solve(v_inhom,v_hom,normalize=False)

            x_grid, y_grid, ds_grid = eit.mask_value(ds,mask_value=np.nan)
            image = np.real(ds_grid)
            image = np.flipud(image)

            ax = axes[row_index, col_index]
            im = ax.imshow(image, interpolation="none", cmap="coolwarm", extent=[-1, 1, -1, 1])
            ax.set_title(f"p={p_val} | lambda={lamb_val}")
            ax.axis('off')
    fig.colorbar(im, ax=axes.ravel().tolist(), label='Conductivity Change')
    plt.show()


def my_jacobian_solver(v_inhom, v_hom ,mesh_input, protocol_input):
    """
        Reconstruct using the jacobian solver aka GN onestep algorithm
        plot reconstructions for different hyper-parameters and priors
    """
    pts = mesh_input.node
    tri = mesh_input.element
    x, y = pts[:,0], pts[:,1]
    #regularization priors
    method_priors = ["kotre", "lm", "dgn" ]

    #regularization weights
    lambda_values = [0.01, 0.1, 1, 10, 100]
    fig, axes = plt.subplots(len(method_priors), len(lambda_values), constrained_layout=True, figsize=(12, 9))
    fig.suptitle('Jacobian Solver')
    for row_index, method in enumerate(method_priors):
        for col_index, lamb_val in enumerate(lambda_values):

            eit_solver = jac.JAC(mesh_input, protocol_input)
            eit_solver.setup(p=0.5, lamb=lamb_val, method=method, perm=1, jac_normalized=True)
            ds = eit_solver.solve(v_inhom,v_hom,normalize=True)
            ds_n = sim2pts(pts, tri, np.real(ds))
            ax = axes[row_index, col_index]
            im = ax.tripcolor(x, y, tri, ds_n, shading="flat", cmap="coolwarm")
            ax.set_title(f"{method} | lambda = {lamb_val}", fontsize=10)
            ax.set_aspect("equal")
            ax.axis('off')

    cbar = fig.colorbar(im, ax=axes.ravel().tolist(), fraction=0.02, pad=0.02)
    cbar.set_label('Conductivity Change (Delta sigma)', rotation=270, labelpad=15)
    plt.show()

if __name__ == "__main__":

    # import voltage meaurements files
    #homogeneous measurements
    V_Homog = np.loadtxt('Saline_Tank_Test10_8E_250ml_2026-03-23/T10-chan8-Tank-DEFAULT-NoObj-F_25kHz-V_250-d_155mm-med_Saline_36.7gL.txt')
    #inhomogeneous measurements
    V_inHomog = np.loadtxt('Saline_Tank_Test10_8E_250ml_2026-03-23/T10-chan8-Tank-CC-F_25kHz-V_250ml-d_155mm.txt')
    print("Data imported")

    # variables
    num_electrodes = 8
    # create 2D mesh
    mesh_obj = mesh.create(n_el=num_electrodes, h0=0.1)
    protocol_obj = protocol.create(n_el=num_electrodes, dist_exc=1, step_meas=1, parser_meas="std")
    #plot voltage measurements
    plot_voltages(V_Homog, V_inHomog)
    print("Voltage plot ready")
    #creat seperate functions for GN_one-step AND GREIT solver for testing
    my_jacobian_solver(V_inHomog, V_Homog, mesh_obj, protocol_obj)
    print("Jacobian reconstruction solver done")
    my_greit_solver(V_inHomog, V_Homog, mesh_obj, protocol_obj)
    print("GREIT solver done")
