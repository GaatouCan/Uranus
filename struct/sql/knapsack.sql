CREATE TABLE `base_item_bag`
(
    `pid` BIGINT NOT NULL COMMENT '玩家ID',
    `grid` BLOB COMMENT '网格',
    PRIMARY KEY (`pid`)
) ENGINE = InnoDB CHARACTER SET = utf16 COMMENT '基础背包数据';