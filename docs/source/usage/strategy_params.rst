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
    | Adaptive_DE_Ooura    | ``n_eval``, ``eps_rel``                    | 0                    |
    +----------------------+--------------------------------------------+----------------------+
    | Fixed_DE_Ogata       | ``n_eval``, ``f_max``                      | 1                    |
    +----------------------+--------------------------------------------+----------------------+
    | all ``DHT_*``        | N/A                                        | 6 to 11              |
    | filters              |                                            |                      |
    +----------------------+--------------------------------------------+----------------------+
    | QWE_Key              | ``n_eval``, ``eps_rel``                    | 12                   |
    +----------------------+--------------------------------------------+----------------------+
    | QWE_Chave            | ``n_eval``, ``eps_rel``                    | 13                   |
    +----------------------+--------------------------------------------+----------------------+
    | FFTLog               | N/A                                        | 16                   |
    +----------------------+--------------------------------------------+----------------------+

See :ref:`strategy-selection` for the full list of ``DHT_*`` filter names.


Here is an explanation of each parameter:

.. doxygenstruct:: strategy_params
   :project: libhankel
   :members:


For the **DHT_*** filters and **FFTLog**, you may supply an empty struct in C (or an empty dict
in Python) for ``strategy_params`` — they require no parameters at all.

**Fixed_DE_Ogata** does require ``n_eval`` and ``f_max``, but does not use ``eps_rel``.

Setting ``eps_rel`` for any of these three strategy families (**DHT_***, **Fixed_DE_Ogata**,
**FFTLog**) will produce a warning on ``stderr``, since ``eps_rel`` is only meaningful for the
adaptive strategies (``Adaptive_DE_Ooura``, ``QWE_Key``, ``QWE_Chave``).
