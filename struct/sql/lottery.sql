CREATE TABLE `lottery`
(
    `pid` BIGINT NOT NULL COMMENT '玩家ID',
    `level` INT NOT NULL DEFAULT 0 COMMENT '等级',
    `level_times` BIGINT NOT NULL DEFAULT 0 COMMENT '等级次数',
    `total_times` BIGINT NOT NULL DEFAULT 0 COMMENT '总共次数',
    `today_times` BIGINT NOT NULL DEFAULT 0 COMMENT '今天次数',
    `wished_times` BIGINT NOT NULL DEFAULT 0 COMMENT '心愿次数',
    `selected` BIGINT NOT NULL DEFAULT 0 COMMENT '心愿选择',
    PRIMARY KEY (`pid`)
) ENGINE = InnoDB CHARACTER SET = utf16 COMMENT '抽奖';

CREATE TABLE `lottery_guarantee`
(
    `pid` BIGINT NOT NULL COMMENT '玩家ID',
    `index` INT NOT NULL DEFAULT 0 COMMENT '索引',
    `count` BIGINT NOT NULL DEFAULT 0 COMMENT '次数',
    PRIMARY KEY (`pid`, `index`)
) ENGINE = InnoDB CHARACTER SET = utf16 COMMENT '抽奖保底';

CREATE TABLE `lottery_copy`
(
    `pid` BIGINT NOT NULL COMMENT '玩家ID',
    `index` INT NOT NULL DEFAULT 0 COMMENT '索引',
    `count` BIGINT NOT NULL DEFAULT 0 COMMENT '次数',
    PRIMARY KEY (`pid`, `index`)
) ENGINE = InnoDB CHARACTER SET = utf16 COMMENT '奖励剩余份数';

CREATE TABLE `lottery_log`
(
    `pid` BIGINT NOT NULL COMMENT '玩家ID',
    `timestamp` BIGINT NOT NULL COMMENT '时间',
    `consume_item_id` INT NOT NULL COMMENT '使用道具',
    `consume_num` INT NOT NULL COMMENT '使用道具数量',
    `instead_item_id` INT NOT NULL COMMENT '代替道具',
    `instead_num` INT NOT NULL COMMENT '代替道具数量',
    `reward` BLOB COMMENT '奖励结果',
    PRIMARY KEY (`pid`, `timestamp`)
) ENGINE = InnoDB CHARACTER SET = utf16 COMMENT '抽奖记录';