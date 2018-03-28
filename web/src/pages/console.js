import React, { Component } from 'react';
import classNames from 'classnames';
import PropTypes from 'prop-types';
import { withStyles } from 'material-ui/styles';
import Button from 'material-ui/Button';
import Input, { InputLabel, InputAdornment } from 'material-ui/Input';
import { FormControl, FormHelperText } from 'material-ui/Form';
import Paper from 'material-ui/Paper';

import './console.css';

const styles = theme => ({
	button: {
		margin: theme.spacing.unit,
	},
	input: {
		display: 'none',
	},
	margin: {
		margin: theme.spacing.unit,
	},
	withoutLabel: {
		marginTop: theme.spacing.unit * 3,
	},
	textField: {
		flexBasis: 200,
	},
	pager: {
		padding: 0,
		overflow: "hidden",
	},
});

class console extends Component {
	msgs = null;
	msgsScroll = null;

	msgsAppend = str => {
		if (this.msgs) this.msgs.insertAdjacentHTML("beforeend", this.colourd(str).replace(/\n/g, "<br>"));
		if (this.msgsScroll) this.msgsScroll.scrollTop = this.msgsScroll.scrollHeight;
	};

	colourd = str => {
		let bold = false, colour = null;
		str = str.replace(/\033\[([0-9;]+)m/g, function (esc, clr) {
			let res = clr.match(/[0-9]+/g);
			for (let i in res) {
				let c = parseInt(res[i], 10);
				if (c === 1)
					bold = true;
				else
					colour = c;
			}
			return "";
		});
		switch (colour) {
			case 30: colour = "#fff"; break;
			case 31: colour = "#bb0000"; break;
			case 32: colour = "#00bb00"; break;
			case 33: colour = "#bbbb00"; break;
			case 34: colour = "#0000bb"; break;
			case 35: colour = "#bb00bb"; break;
			case 36: colour = "#00bbbb"; break;
			case 37: colour = "#bbbbbb"; break;
			case 90: colour = "#555555"; break;
			case 91: colour = "#ff5555"; break;
			case 92: colour = "#55ff55"; break;
			case 93: colour = "#ffff55"; break;
			case 94: colour = "#5555ff"; break;
			case 95: colour = "#ff55ff"; break;
			case 96: colour = "#55ffff"; break;
			case 97: colour = "#000"; break;
			default: colour = null; break;
		}
		let e = document.createElement("span");
		e.innerText = str;
		if (bold === true)
			e.style["font-weight"] = "bold";
		if (colour !== null)
			e.style["color"] = colour;
		return e.outerHTML;
	}

	state = {

	};

	componentDidMount() {
		let url;
		if (process.env.NODE_ENV === 'development')
			url = "ws://localhost:25580";
		else
			url = "ws://" + window.location.hostname + ":" + window.location.port;
		this.ws = new WebSocket(url, "web-console");
		this.ws.onopen = () => { this.msgsAppend("WebSocket connected\n"); }
		this.ws.onclose = () => { this.msgsAppend("WebSocket disconnected\n"); }
		this.ws.onmessage = (msg) => { this.msgsAppend(msg.data); }
	}

	render() {
		const { classes } = this.props;

		return (
			<div id="console">
				<Paper className={classes.pager} elevation={4}>
					<div className="msgsScroll" ref={msgsScroll => { this.msgsScroll = msgsScroll; }}>
						<div className="msgs" ref={msgs => { this.msgs = msgs; }}></div>
					</div>
				</Paper>
				<FormControl fullWidth className={classes.margin}>
					<InputLabel htmlFor="command">servmc></InputLabel>
					<Input
						id="command"
						onKeyUp={event => {
							if (event.keyCode === 13) {
								this.ws.send(event.target.value);
								event.target.value = "";	//清空文本框
							}
						}}
					/>
				</FormControl>
			</div>
		);
	}
}

console.propTypes = {
	classes: PropTypes.object.isRequired,
};

export default withStyles(styles)(console);
// export default console;