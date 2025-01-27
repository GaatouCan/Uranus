CREATE TABLE `command`
(
    `id` BIGINT UNSIGNED NOT NULL COMMENT '命令ID',
    `creator` TEXT COMMENT '创建者',
    `type` VARCHAR(255) COMMENT '命令类型',
    `param` VARCHAR(255) COMMENT '命令参数',
    `create_time` BIGINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '创建时间',
    `finish_time` BIGINT UNSIGNED NOT NULL DEFAULT 0 COMMENT '完成时间',
    `extend` TEXT COMMENT '扩展(备注或说明)',
    PRIMARY KEY (`id`)
) ENGINE = InnoDB CHARACTER SET = utf16 COMMENT '运营命令';