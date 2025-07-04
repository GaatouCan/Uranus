CREATE TABLE `player_cache`
(
    `pid` BIGINT NOT NULL COMMENT '玩家ID',
    `cache` BLOB COMMENT '缓存',
    PRIMARY KEY (`pid`)
) ENGINE = InnoDB CHARACTER SET = utf16 COMMENT '玩家缓存';