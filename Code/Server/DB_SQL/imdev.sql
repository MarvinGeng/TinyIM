/*
 Navicat Premium Data Transfer

 Source Server         : Local
 Source Server Type    : MySQL
 Source Server Version : 80016
 Source Host           : 127.0.0.1:3306
 Source Schema         : imdev

 Target Server Type    : MySQL
 Target Server Version : 80016
 File Encoding         : 65001

 Date: 22/10/2019 15:39:30
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for t_add_friend_msg
-- ----------------------------
DROP TABLE IF EXISTS `t_add_friend_msg`;
CREATE TABLE `t_add_friend_msg`  (
  `F_INDEX` int(255) NOT NULL AUTO_INCREMENT,
  `F_MSG_ID` char(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL COMMENT '消息ID',
  `F_USER_ID` char(8) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL COMMENT '请求添加好友的用户',
  `F_FRIEND_ID` char(8) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL COMMENT '被添加好友的用户',
  `F_ADD_FRIEND_STATUS` enum('UN_KNOWN','UN_READ','READ_UN_NOTIFY','NOTIFY') CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT 'UN_KNOWN' COMMENT '添加消息的状态，\r\n未读----->已读未通知------>已通知',
  `F_FRIEND_OPTION` enum('UN_KNOWN','AGREE_ADD','REFUSE_ADD') CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT 'UN_KNOWN' COMMENT '同意，未同意',
  `F_CREATE_TIME` datetime(0) DEFAULT NULL COMMENT '创建的时间',
  `F_OPTION_TIME` datetime(0) DEFAULT NULL COMMENT '对方回复的时间',
  `F_NOTIFY_TIME` datetime(0) DEFAULT NULL COMMENT '通知发送方的时间',
  PRIMARY KEY (`F_INDEX`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for t_add_invate_group_relation_msg
-- ----------------------------
DROP TABLE IF EXISTS `t_add_invate_group_relation_msg`;
CREATE TABLE `t_add_invate_group_relation_msg`  (
  `F_INDEX` int(255) NOT NULL,
  `F_MSG_ID` char(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL,
  `F_USER_ID` char(8) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `F_GROUP_ID` char(8) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `F_CREATE_TIME` datetime(0) DEFAULT NULL,
  `F_OPTION_TIME` datetime(0) DEFAULT NULL,
  `F_NOTIFY_TIME` datetime(0) DEFAULT NULL,
  PRIMARY KEY (`F_INDEX`, `F_MSG_ID`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for t_friend_chat_msg
-- ----------------------------
DROP TABLE IF EXISTS `t_friend_chat_msg`;
CREATE TABLE `t_friend_chat_msg`  (
  `F_INDEX` bigint(255) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '自增ID，由数据库产生',
  `F_MSG_ID` char(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT '消息ID,由程序生成',
  `F_MSG_TYPE` char(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT '消息类型',
  `F_FROM_ID` char(8) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT '消息发送者ID',
  `F_TO_ID` char(8) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT '消息接收者ID',
  `F_MSG_CONTEXT` text CHARACTER SET utf8 COLLATE utf8_bin NOT NULL COMMENT '消息内容',
  `F_OTHER_INFO` text CHARACTER SET utf8 COLLATE utf8_bin COMMENT '消息的其他部分的信息，比如文本消息的字体信息',
  `F_READ_FLAG` enum('UNREAD','READ') CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT 'UNREAD' COMMENT '信息是否被读取,\'READ\',\'UNREAD\'',
  `F_CREATE_TIME` timestamp(0) DEFAULT CURRENT_TIMESTAMP COMMENT '信息创建时间',
  `F_READ_TIME` timestamp(0) DEFAULT NULL COMMENT '信息读取时间',
  PRIMARY KEY (`F_INDEX`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for t_friend_relation
-- ----------------------------
DROP TABLE IF EXISTS `t_friend_relation`;
CREATE TABLE `t_friend_relation`  (
  `F_INDEX` int(255) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '自增ID',
  `F_USER_ID` char(8) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT '用户ID,外键到T_USER表的F_USER_ID',
  `F_TEAM_ID` char(8) CHARACTER SET utf8 COLLATE utf8_bin DEFAULT '1000000' COMMENT '分组ID',
  `F_TEAM_NAME` char(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL COMMENT '分组名',
  `F_FRIEND_ID` char(8) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT '好友ID,外键到T_USER表的F_USER_ID',
  `F_STATUS` enum('FRIEND','BLACK') CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL COMMENT '好友状态,\'FRIEND\',\'BLACK\'',
  `F_CREATE_TIME` datetime(0) DEFAULT NULL COMMENT '创建时间',
  `F_UPDATE_TIME` datetime(0) DEFAULT NULL COMMENT '更新时间',
  PRIMARY KEY (`F_INDEX`, `F_USER_ID`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for t_group
-- ----------------------------
DROP TABLE IF EXISTS `t_group`;
CREATE TABLE `t_group`  (
  `F_INDEX` int(255) NOT NULL AUTO_INCREMENT COMMENT '自增ID',
  `F_GROUP_ID` char(8) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT '群组ID',
  `F_GROUP_NAME` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT 'https://www.dennisthink.com' COMMENT '群组名称',
  `F_CREATE_TIME` datetime(0) DEFAULT NULL COMMENT '创建时间',
  `F_GROUP_INFO` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT 'https://www.dennisthink.com' COMMENT '群组简介',
  PRIMARY KEY (`F_INDEX`, `F_GROUP_ID`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for t_group_chat_msg
-- ----------------------------
DROP TABLE IF EXISTS `t_group_chat_msg`;
CREATE TABLE `t_group_chat_msg`  (
  `F_INDEX` int(255) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '自增ID',
  `F_MSG_ID` char(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL COMMENT '消息ID,由程序生成 ',
  `F_MSG_TYPE` char(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL COMMENT '消息类型',
  `F_SENDER_ID` char(8) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL COMMENT '消息发送者ID,外键链接到T_USER的F_USER_ID',
  `F_GROUP_ID` char(8) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL COMMENT '群组ID，外键链接到T_GROUP的F_GROUP_ID',
  `F_MSG_CONTEXT` blob COMMENT '消息内容',
  `F_OTHER_INFO` char(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT NULL,
  `F_CREATE_TIME` datetime(0) DEFAULT NULL COMMENT '创建时间',
  PRIMARY KEY (`F_INDEX`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for t_group_relation
-- ----------------------------
DROP TABLE IF EXISTS `t_group_relation`;
CREATE TABLE `t_group_relation`  (
  `F_INDEX` int(255) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '自增ID',
  `F_GROUP_ID` char(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT '群组ID',
  `F_USER_ID` char(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT '用户ID',
  `F_ROLE_TYPE` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT '用户角色，\'OWNER\',\'MANAGER\',\'MEMBER\' ',
  `F_LAST_READ_MSG_ID` char(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT '00000000' COMMENT '阅读的最后一个消息的编号',
  `F_CREATE_TIME` datetime(0) DEFAULT NULL COMMENT '创建时间',
  PRIMARY KEY (`F_INDEX`) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for t_user
-- ----------------------------
DROP TABLE IF EXISTS `t_user`;
CREATE TABLE `t_user`  (
  `F_INDEX` int(255) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '数据记录索引，自增',
  `F_USER_ID` char(8) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT '00000000' COMMENT '用户ID',
  `F_USER_NAME` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT '11111111' COMMENT '用户名',
  `F_PASS_WORD` varchar(128) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL DEFAULT '11111111' COMMENT '用户密码',
  `F_ADDRESS` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT 'Test_User_Address' COMMENT '用户住址',
  `F_BIRTH_DATE` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT '19901010' COMMENT '出生日期',
  `F_EMAIL_ADDR` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT 'test@dennisthink.com' COMMENT '电子邮箱地址',
  `F_NICK_NAME` varchar(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT 'DennisThink.com' COMMENT '用户昵称',
  `F_CREATE_TIME` timestamp(0) DEFAULT NULL COMMENT '创建时间',
  `F_UPDATE_TIME` timestamp(0) DEFAULT NULL COMMENT '最后更新时间',
  `F_SIGNATURE` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT '个人博客 https://www.dennisthink.com/' COMMENT '用户签名',
  `F_ON_LINE_STATE` enum('ON_LINE','OFF_LINE') CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT 'OFF_LINE' COMMENT '用户在线状态，分为在线和离线',
  `F_FACE_ID` int(11) DEFAULT 2 COMMENT '头像编号',
  `F_GENDER` enum('MALE','FEMALE','UN_KNOWN') CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT 'UN_KNOWN' COMMENT '用户性别',
  PRIMARY KEY (`F_INDEX`, `F_USER_ID`) USING BTREE,
  UNIQUE INDEX `F_USER_NAME_INDEX`(`F_USER_NAME`) USING BTREE COMMENT '用户名做唯一索引'
) ENGINE = InnoDB AUTO_INCREMENT = 1 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin ROW_FORMAT = Dynamic;

-- ----------------------------
-- Table structure for t_user_team
-- ----------------------------
DROP TABLE IF EXISTS `t_user_team`;
CREATE TABLE `t_user_team`  (
  `F_INDEX` int(255) NOT NULL AUTO_INCREMENT COMMENT '唯一ID',
  `F_USER_ID` char(8) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT '用户ID',
  `F_TEAM_ID` char(8) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin NOT NULL COMMENT '分组编号',
  `F_TEAM_NAME` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_bin DEFAULT '我的好友' COMMENT '分组名称',
  `F_CREATE_TIME` datetime(0) DEFAULT NULL COMMENT '创建日期',
  PRIMARY KEY (`F_INDEX`) USING BTREE
) ENGINE = InnoDB CHARACTER SET = utf8mb4 COLLATE = utf8mb4_bin ROW_FORMAT = Dynamic;

SET FOREIGN_KEY_CHECKS = 1;
