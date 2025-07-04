CREATE TABLE `appear`
(
    `pid` BIGINT NOT NULL COMMENT '玩家ID',
    `avatar` INT NOT NULL DEFAULT 0 COMMENT '头像',
    `avatar_frame` INT NOT NULL DEFAULT 0 COMMENT '头像框',
    `update_time` BIGINT NOT NULL DEFAULT 0 COMMENT '更新时间',
    PRIMARY KEY (`pid`)
) ENGINE = InnoDB CHARACTER SET = utf16 COMMENT '玩家外观';

CREATE TABLE `avatar`
(
    `pid` BIGINT NOT NULL COMMENT '玩家ID',
    `index` INT NOT NULL DEFAULT 0 COMMENT '索引',
    `expired_time` BIGINT NOT NULL DEFAULT 0 COMMENT '过期时间',
    `activated` BOOLEAN NOT NULL DEFAULT FALSE COMMENT '是否激活',
    PRIMARY KEY (`pid`, `index`)
) ENGINE = InnoDB CHARACTER SET = utf16 COMMENT '玩家头像';

CREATE TABLE `avatar_frame`
(
    `pid` BIGINT NOT NULL COMMENT '玩家ID',
    `index` INT NOT NULL DEFAULT 0 COMMENT '索引',
    `expired_time` BIGINT NOT NULL DEFAULT 0 COMMENT '过期时间',
    `activated` BOOLEAN NOT NULL DEFAULT FALSE COMMENT '是否激活',
    PRIMARY KEY (`pid`, `index`)
) ENGINE = InnoDB CHARACTER SET = utf16 COMMENT '玩家头像框';