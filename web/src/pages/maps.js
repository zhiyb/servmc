import React, { Component } from 'react';

import diamond_ore from './maps/diamond_ore.png'
import emerald_ore from './maps/emerald_ore.png'
import stone from './maps/stone.png'

import './maps.css';

class maps extends Component {
	canvas = null;
	canvasContext = null;
	images = {};

	mapScale = 1;

	getblock = (x, y) => {
		switch (Math.round(Math.random() * 10)) {		//目前为随机，未来将会改为实际的获取
			case 1:
				return this.images.diamond_ore;
			case 2:
				return this.images.emerald_ore;
			default:
				return this.images.stone;
		}
	}

	draw = () => {
		if (this.canvas) {
			this.canvasContext.clearRect(0, 0, this.canvas.width, this.canvas.height)
			for (let x = 0; x < 100; x++)for (let y = 0; y < 100; y++) {
				this.canvasContext.drawImage(this.getblock(x, y), 0, 0, 16, 16, x * 16 * this.mapScale, y * 16 * this.mapScale, 16 * this.mapScale, 16 * this.mapScale);
			}
		}
	}

	zoomIn = () => {
		this.mapScale *= 1.1;
	}

	zoomOut = () => {
		this.mapScale /= 1.1
	}

	componentDidMount() {
		(this.images.diamond_ore = new Image()).src = diamond_ore;
		(this.images.emerald_ore = new Image()).src = emerald_ore;
		(this.images.stone = new Image()).src = stone;
	}

	render() {
		return (
			<div>
				<button onClick={this.draw}>draw</button>
				<button onClick={this.zoomIn}>+</button>
				<button onClick={this.zoomOut}>-</button>
				<br />
				<canvas
					className="canvas"
					ref={canvas => { this.canvas = canvas; this.canvasContext = canvas ? canvas.getContext("2d") : null; }}
					height="1000px"
					width="1000px"
				></canvas>
			</div >
		);
	}
}

export default maps;