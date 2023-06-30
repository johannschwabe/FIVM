IMPORT DTREE FROM FILE 'tpch_4-Q3.txt';

CREATE DISTRIBUTED TYPE RingFactorizedRelation
FROM FILE 'ring/ring_factorized.hpp'
WITH PARAMETER SCHEMA (dynamic_min);

CREATE STREAM orders (
        orderkey         INT,
        custkey          INT,
        o_orderstatus    CHAR(1),
        o_totalprice     DECIMAL,
        o_orderdate      CHAR(10),
        o_orderpriority  CHAR(15),
        o_clerk          CHAR(15),
        o_shippriority   INT,
        o_comment        VARCHAR(79)
    )
  FROM FILE './datasets/tpch_unordered1/orders.csv'
  LINE DELIMITED CSV (delimiter := '|');


CREATE STREAM customer (
        custkey        INT,
        c_name         VARCHAR(25),
        c_address      VARCHAR(40),
        nationkey      INT,
        c_phone        CHAR(15),
        c_acctbal      DECIMAL,
        c_mktsegment   CHAR(10),
        c_comment      VARCHAR(117)
    )
  FROM FILE './datasets/tpch_unordered1/customer.csv'
  LINE DELIMITED CSV (delimiter := '|');


SELECT SUM(
    [lift<0>: RingFactorizedRelation<[0, INT]>](custkey) *
    [lift<1>: RingFactorizedRelation<[1, INT]>](orderkey) *
    [lift<8>: RingFactorizedRelation<[8, INT]>](nationkey)
   )
FROM orders NATURAL JOIN customer;