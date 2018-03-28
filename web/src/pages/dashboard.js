import React, { Component } from 'react';
import PropTypes from 'prop-types';
import { withStyles } from 'material-ui/styles';
import { CircularProgress } from 'material-ui/Progress';
import Paper from 'material-ui/Paper';
import Grid from 'material-ui/Grid';
import Typography from 'material-ui/Typography';

import query from '../query';

const styles = theme => ({
	root: {
		flexGrow: 1,
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
		version: null,
	};

	componentDidMount() {
		query({"type":"version"}).then((ret) => {
			this.setState({version: ret})
		})
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
										玩家事件
									</Typography>
									<pre>
										10:01:00 YJBeetle 加入游戏
										<br/>
										10:02:02 YJBeetle 离开游戏
										<br/>
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
										<br/>
										10:01:42 YJBeetle: test msg
										<br/>
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
										在线人数
									</Typography>
									<CircularProgress className={classes.progress} variant="static" value={19} size={80} />
									<Typography variant="headline" component="h2">
										30/160
									</Typography>
								</Paper>
							</Grid>
							<Grid item>
								<Paper className={classes.paper}>
									<Typography className={classes.title} color="textSecondary">
										备份状态
									</Typography>
									<Typography variant="headline" component="h2">
										最后备份于<br/>xxxx
									</Typography>
								</Paper>
							</Grid>
							<Grid item>
								<Paper className={classes.paper}>
									<Typography className={classes.title} color="textSecondary">
										游戏版本
									</Typography>
									<Typography variant="headline" component="h2">
										{this.state.version?this.state.version:"查询中……"}
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
									<CircularProgress className={classes.progress} variant="static" value={75} size={80} />
									<Typography variant="headline" component="h2">
									75%
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