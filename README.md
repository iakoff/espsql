http://iak0ff.blogspot.com/p/esp-sql.html


--
-- Table structure for table `rawacts`
--

DROP TABLE IF EXISTS `rawacts`;
CREATE TABLE `rawacts` (
  `idrawacts` int(11) NOT NULL AUTO_INCREMENT,
  `pinvalue` varchar(45) DEFAULT NULL,
  `pinname` varchar(45) DEFAULT NULL,
  `stamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `updated` int(11) DEFAULT NULL,
  `device` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`idrawacts`,`stamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `rawdata`;
CREATE TABLE `rawdata` (
  `idrecord` int(11) NOT NULL AUTO_INCREMENT,
  `value_name` varchar(45) DEFAULT NULL,
  `value_value` varchar(45) DEFAULT NULL,
  `stamp` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `device` varchar(45) DEFAULT NULL,
  PRIMARY KEY (`idrecord`,`stamp`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
