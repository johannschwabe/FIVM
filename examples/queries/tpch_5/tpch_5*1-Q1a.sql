IMPORT DTREE FROM FILE 'tpch_5-Q1a.txt';

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
  FROM FILE './datasets/tpch1/customer.csv'
  LINE DELIMITED CSV (delimiter := '|');


CREATE
STREAM part (
        partkey        INT,
        p_name         VARCHAR(55),
        p_mfgr         CHAR(25),
        p_brand        CHAR(10),
        p_type         VARCHAR(25),
        p_size         INT,
        p_container    CHAR(10),
        p_retailprice  DECIMAL,
        p_comment      VARCHAR(23)
    )
  FROM FILE './datasets/tpch1/part.csv'
  LINE DELIMITED CSV (delimiter := '|');


CREATE STREAM partsupp (
        partkey         INT,
        suppkey         INT,
        ps_availqty     INT,
        ps_supplycost   DECIMAL,
        ps_comment      VARCHAR(199)
    )
  FROM FILE './datasets/tpch1/partsupp.csv'
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
  FROM FILE './datasets/tpch1/supplier.csv'
  LINE DELIMITED CSV (delimiter := '|');

CREATE STREAM nation (
        nationkey      INT,
        n_name         CHAR(25),
        regionkey    INT,
        n_comment      VARCHAR(152)
)
    FROM FILE './datasets/tpch1/nation.csv'
  LINE DELIMITED CSV (delimiter := '|');

SELECT SUM(
    [lift<0>: RingFactorizedRelation<[0, INT]>](suppkey) *
    [lift<1>: RingFactorizedRelation<[1, INT]>](partkey) *
    [lift<2>: RingFactorizedRelation<[2, INT]>](nationkey) *
    [lift<3>: RingFactorizedRelation<[3, CHAR(25)]>](n_name) *
    [lift<6>: RingFactorizedRelation<[6, CHAR(25)]>](s_name) *
    [lift<11>: RingFactorizedRelation<[11, VARCHAR(55)]>](p_name) *
    [lift<18>: RingFactorizedRelation<[18, INT]>](ps_availqty)
   )
FROM part NATURAL JOIN customer NATURAL JOIN partsupp NATURAL JOIN supplier NATURAL JOIN nation;