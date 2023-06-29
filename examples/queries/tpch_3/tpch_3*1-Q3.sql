IMPORT DTREE FROM FILE 'tpch_3-Q3.txt';

CREATE DISTRIBUTED TYPE RingFactorizedRelation
FROM FILE 'ring/ring_factorized.hpp'
WITH PARAMETER SCHEMA (dynamic_min);

CREATE
STREAM supplier (
        suppkey        INT,
        s_name         VARCHAR(25),
        s_address         CHAR(25),
        nationkey        INT,
        s_phone         CHAR(15),
        s_acctbal         DECIMAL,
        s_comment         VARCHAR(101)
    )
  FROM FILE './datasets/jcch_unordered1/part.csv'
  LINE DELIMITED CSV (delimiter := '|');

CREATE
STREAM partsupp (
        partkey         INT,
        suppkey         INT,
        ps_availqty     INT,
        ps_supplycost   DECIMAL,
        ps_comment      VARCHAR(199)
    )
  FROM FILE './datasets/jcch_unordered1/partsupp.csv'
  LINE DELIMITED CSV (delimiter := '|');

SELECT SUM(
    [lift<0>: RingFactorizedRelation<[0, INT]>](suppkey) *
    [lift<1>: RingFactorizedRelation<[1, INT,INT, DECIMAL]>](partkey,ps_availqty, ps_supplycost) *
    [lift<4>: RingFactorizedRelation<[4, VARCHAR(25)]>](s_name)
)
FROM supplier NATURAL JOIN partsupp;

