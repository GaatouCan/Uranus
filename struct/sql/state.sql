CREATE TABLE `state`
(
    `pid` BIGINT NOT NULL COMMENT '玩家ID',
    `level` INT NOT NULL DEFAULT 0 COMMENT '等级',
    `experience` BIGINT NOT NULL DEFAULT 0 COMMENT '经验值',
    `last_login_time` BIGINT NOT NULL DEFAULT 0 COMMENT '最后登陆时间',
    `last_logout_time` BIGINT NOT NULL DEFAULT 0 COMMENT '最后登出时间',
    PRIMARY KEY (`pid`)
) ENGINE = InnoDB CHARACTER SET = utf16 COMMENT '玩家状态';