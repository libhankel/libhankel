.. _strategy-parameters:

Strategy Parameters
================================

The ``hankel_transform`` function is designed to receive a both a ``strategy_name``, 
which allows the user the select one of the strategies available, and ``strategy_params``, 
which will contain the parameters required by the particular strategy being used. 
The latter will be a dict when using the Python API and a struct when using the C API.

The table below reports the parameters required by each strategy.


.. table:: Parameters needed by each strategy.

    +----------------------+--------------------------------------------+----------------------+
    | Strategy Name        | Field names needed in ``strategy_params``  | Index (as in SASfit) |
    +======================+============================================+======================+
    | Adaptive_DE_Ooura    | ``bessel_zero_idx``, ``eps_rel``           | 0                    |
    +----------------------+--------------------------------------------+----------------------+
    | Fixed_DE_Ogata       | ``n_nodes``, ``f_max``                     | 1                    |
    +----------------------+--------------------------------------------+----------------------+
    | all ``DHT_*``        | N/A                                        | 6 to 11              |
    | filters              |                                            |                      |
    +----------------------+--------------------------------------------+----------------------+
    | QWE_Key              | ``max_iters``, ``eps_rel``                 | 12                   |
    +----------------------+--------------------------------------------+----------------------+
    | QWE_Chave            | ``max_iters``, ``eps_rel``                 | 13                   |
    +----------------------+--------------------------------------------+----------------------+
    | FFTLog               | N/A                                        | 16                   |
    +----------------------+--------------------------------------------+----------------------+

See :ref:`strategy-selection` for the full list of ``DHT_*`` filter names.

The struct has three separate integer fields; set only the one that matches the strategy you
are calling (leave the others at zero or unset):

- ``Adaptive_DE_Ooura`` → ``bessel_zero_idx``: index of the Bessel zero used to split the
  integral at the boundary between the finite and oscillatory parts. Capped internally at 10.
  Corresponds to ``N_ogata`` in SASfit.
- ``Fixed_DE_Ogata`` → ``n_nodes``: number of quadrature nodes. More nodes give higher
  accuracy at greater cost. Corresponds to ``N_ogata`` in SASfit.
- ``QWE_Key`` / ``QWE_Chave`` → ``max_iters``: maximum number of QWE extrapolation iterations
  before the loop stops.

Here is an explanation of each parameter:

.. doxygenstruct:: strategy_params
   :project: libhankel
   :members:


For the ``DHT_*`` filters and ``FFTLog``, you may supply an empty struct in C (or an empty dict
in Python) for ``strategy_params`` — they require no parameters at all.

``Fixed_DE_Ogata`` does require ``n_nodes`` and ``f_max``, but does not use ``eps_rel``.

Setting ``eps_rel`` for any of these three strategy families (``DHT_*``, ``Fixed_DE_Ogata``,
``FFTLog``) will produce a warning on ``stderr``, since ``eps_rel`` is only meaningful for the
adaptive strategies (``Adaptive_DE_Ooura``, ``QWE_Key``, ``QWE_Chave``).
