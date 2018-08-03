import React, { Component } from 'react';
import PropTypes from 'prop-types';
import { withStyles } from 'material-ui/styles';
import { CircularProgress } from 'material-ui/Progress';
import Paper from 'material-ui/Paper';
import Grid from 'material-ui/Grid';
import Typography from 'material-ui/Typography';
import dateFormat from 'dateformat';

import query from '../query';

const styles = theme => ({
	root: {
		flexGrow: 1,
		padding: theme.spacing.unit * 3,
		overflow: 'auto',
	},
	paper: {
		padding: theme.spacing.unit * 2,
		textAlign: 'center',
		color: theme.palette.text.secondary,
	},
	paper1: {
		padding: theme.spacing.unit * 2,
		color: theme.palette.text.secondary,
	},
	progressbg: {
		margin: theme.spacing.unit * 2,
		position: "absolute",
	},
	progress: {
		margin: theme.spacing.unit * 2,
	},
	title: {
		marginBottom: 16,
		fontSize: 14,
	},
});

class dashboard extends Component {

	state = {
		version: undefined,
		players: undefined,
		backup: undefined,
		restart: undefined,
		system: undefined,
	};

	update = () => {
		query({ "action": "query", "type": "version" }).then((ret) => {
			this.setState({ version: ret })
		});
		query({ "action": "query", "type": "players" }).then((ret) => {
			this.setState({ players: ret })
		});
		query({ "action": "query", "type": "backup" }).then((ret) => {
			this.setState({ backup: ret })
		});
		query({ "action": "query", "type": "restart" }).then((ret) => {
			this.setState({ restart: ret })
		});
		query({ "action": "query", "type": "system" }).then((ret) => {
			this.setState({ system: ret })
		});
		this.updateTimer = setTimeout(this.update, 3000);
	}

	componentDidMount() {
		this.update();
	}

	componentWillUnmount() {
		this.updateTimer && clearTimeout(this.updateTimer);
	}

	render() {
		const { classes } = this.props;

		return (
			<div className={classes.root}>

				<Grid container spacing={24}>
					<Grid item xs={12} sm={6}>
						<Grid container
							alignItems="stretch"
							direction="column"
							justify="space-between"
						>
							<Grid item>
								<Paper className={classes.paper1}>
									<Typography className={classes.title} color="textSecondary">
										服务器事件
								</Typography>
									<pre>
										10:01:00 备份完毕
									<br />
										10:02:02 警告信息
								</pre>
								</Paper>
							</Grid>
							<Grid item>
								<Paper className={classes.paper1}>
									<Typography className={classes.title} color="textSecondary">
										玩家事件
									</Typography>
									<pre>
										10:01:00 YJBeetle 加入游戏
										<br />
										10:02:02 YJBeetle 离开游戏
										<br />
										10:10:35 zhiyb 离开游戏
									</pre>
								</Paper>
							</Grid>
							<Grid item>
								<Paper className={classes.paper1}>
									<Typography className={classes.title} color="textSecondary">
										玩家聊天
									</Typography>
									<pre>
										10:01:34 zhiyb: test msg
										<br />
										10:01:42 YJBeetle: test msg
										<br />
										10:01:56 YJBeetle: test msg
									</pre>
								</Paper>
							</Grid>
						</Grid>
					</Grid>
					<Grid item xs={6} sm={3}>
						<Grid container
							alignItems="stretch"
							direction="column"
							justify="space-between"
						>
							<Grid item>
								<Paper className={classes.paper}>
									<Typography className={classes.title} color="textSecondary">
										服务器名
								</Typography>
									<Typography variant="headline" component="h2">
										xxxx
								</Typography>
								</Paper>
							</Grid>
							<Grid item>
								<Paper className={classes.paper}>
									<Typography className={classes.title} color="textSecondary">
										服务器状态
									</Typography>
									<Typography variant="headline" component="h2">
										{
											this.state.restart ?
											this.state.restart.status === "stopped" ? "已停止" :
											this.state.restart.status === "starting" ? "正在启动" :
											this.state.restart.status === "running" ? "运行中" :
											this.state.restart.status === "pending" ? "即将重启" :
											this.state.restart.status === "scheduled" ? "重启已计划：" + dateFormat((new Date(this.state.restart.time * 1000)), "yyyy-mm-dd HH:MM:ss"):
											this.state.restart.status :
											"查询中……"
										}
									</Typography>
								</Paper>
							</Grid>
							<Grid item>
								<Paper className={classes.paper}>
									<Typography className={classes.title} color="textSecondary">
										在线人数
									</Typography>
									<CircularProgress className={classes.progressbg} variant="static" color="inherit" value="100" size={80} />
									<CircularProgress className={classes.progress} variant="static" max={this.state.players ? this.state.players.max : 1} value={this.state.players ? this.state.players.online : 1} size={80} />
									<Typography variant="headline" component="h2">
										{this.state.players ? this.state.players.online : "?"}/{this.state.players ? this.state.players.max : "?"}
									</Typography>
								</Paper>
							</Grid>
							<Grid item>
								<Paper className={classes.paper}>
									<Typography className={classes.title} color="textSecondary">
										定时备份状态
									</Typography>
									<Typography variant="headline" component="h2">
										{
											this.state.backup ?
											this.state.backup.status === "idle" ? "空闲" :
											this.state.backup.status === "scheduled" ? "计划备份：" + dateFormat((new Date(this.state.backup.time * 1000)), "yyyy-mm-dd HH:MM:ss"):
											this.state.backup.status === "active" ? "正在进行备份" :
											this.state.backup.status :
											"查询中……"
										}
									</Typography>
          							<br />
									<Typography className={classes.title} color="textSecondary">
										最后备份于
									</Typography>
									<Typography variant="headline" component="h2">
										{
											this.state.backup ? this.state.backup.last ? dateFormat((new Date(this.state.backup.last * 1000)), "yyyy-mm-dd HH:MM:ss") : "未进行备份" : "查询中……"
										}
									</Typography>
								</Paper>
							</Grid>
							<Grid item>
								<Paper className={classes.paper}>
									<Typography className={classes.title} color="textSecondary">
										游戏版本
									</Typography>
									<Typography variant="headline" component="h2">
										{this.state.version ? this.state.version : "查询中……"}
									</Typography>
								</Paper>
							</Grid>
							<Grid item>
								<Paper className={classes.paper}>
									<Typography className={classes.title} color="textSecondary">
										运行时间
									</Typography>
									<Typography variant="headline" component="h2">
										xxx
									</Typography>
								</Paper>
							</Grid>
						</Grid>
					</Grid>
					<Grid item xs={6} sm={3}>
						<Grid container
							alignItems="stretch"
							direction="column"
							justify="space-between"
						>
							<Grid item>
								<Paper className={classes.paper}>
									<Typography className={classes.title} color="textSecondary">
										内存使用率
									</Typography>
									<CircularProgress className={classes.progressbg} variant="static" color="inherit" value="100" size={80} />
									<CircularProgress className={classes.progress} variant="static" value={67} size={80} />
									<Typography variant="headline" component="h2">
										67%
									</Typography>
								</Paper>
							</Grid>
							<Grid item>
								<Paper className={classes.paper}>
									<Typography className={classes.title} color="textSecondary">
										CPU使用率
									</Typography>
									<Typography variant="headline" component="h2">
										{
											this.state.system ?
											this.state.system.cpus ?
											this.state.system.cpus.map((value, index) => {
												if (value[0] === "cpu")
													return "CPU:" + Math.round(value[2] / value[1]) + "%";
												else
													return "核心" + index + ":" + Math.round(value[2] / value[1]) + "%";
											}) :
											"等待中……" :
											"查询中……"
										}
									</Typography>
								</Paper>
							</Grid>
							<Grid item>
								<Paper className={classes.paper}>
									<Typography className={classes.title} color="textSecondary">
										平均负载
									</Typography>
									<Typography variant="headline" component="h2">
										1.1  1.3  1.71
									</Typography>
								</Paper>
							</Grid>
							<Grid item>
								<Paper className={classes.paper}>
									<Typography className={classes.title} color="textSecondary">
										开机时间
									</Typography>
									<Typography variant="headline" component="h2">
										1d 2h 36min
									</Typography>
								</Paper>
							</Grid>
							<Grid item>
								<Paper className={classes.paper}>
									<Typography className={classes.title} color="textSecondary">
										ServMC版本号
									</Typography>
									<Typography variant="headline" component="h2">
										0.1 - Alpha
									</Typography>
								</Paper>
							</Grid>
						</Grid>
					</Grid>
				</Grid>
			</div>
		);
	}

	static propTypes = {
		classes: PropTypes.object.isRequired,
	};
}


export default withStyles(styles)(dashboard);