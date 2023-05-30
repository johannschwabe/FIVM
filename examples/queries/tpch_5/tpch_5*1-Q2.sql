IMPORT DTREE FROM FILE 'tpch_5-Q2.txt';

CREATE DISTRIBUTED TYPE RingFactorizedRelation
FROM FILE 'ring/ring_factorized.hpp'
WITH PARAMETER SCHEMA (dynamic_min);

CREATE STREAM customer (
        custkey        INT,
        c_name         VARCHAR(25),
        c_address      VARCHAR(40),
        nationkey    INT,
        c_phone        CHAR(15),
        c_acctbal      DECIMAL,
        c_mktsegment   CHAR(10),
        c_comment      VARCHAR(117)
    )
  FROM FILE './datasets/tpch/customer.csv'
  LINE DELIMITED CSV (delimiter := '|');

CREATE STREAM supplier (
        suppkey        INT,
        s_name         CHAR(25),
        s_address      VARCHAR(40),
        nationkey    INT,
        s_phone        CHAR(15),
        s_acctbal      DECIMAL,
        s_comment      VARCHAR(101)
    )
  FROM FILE './datasets/tpch/supplier.csv'
  LINE DELIMITED CSV (delimiter := '|');

CREATE STREAM nation (
        nationkey      INT,
        n_name         CHAR(25),
        regionkey    INT,
        n_comment      VARCHAR(152)
)
    FROM FILE './datasets/tpch/nation.csv'
  LINE DELIMITED CSV (delimiter := '|');

SELECT SUM(
    [lift<0>: RingFactorizedRelation<[0, INT]>](nationkey) *
    [lift<1>: RingFactorizedRelation<[1, INT, CHAR(25), VARCHAR(40)]>](suppkey, s_name, s_address) *
    [lift<4>: RingFactorizedRelation<[4, INT]>](custkey) *
    [lift<10>: RingFactorizedRelation<[10, CHAR(25)]>](n_name)
   )
FROM  customer NATURAL JOIN supplier NATURAL JOIN nation;