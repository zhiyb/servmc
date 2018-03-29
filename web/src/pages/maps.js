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
	mapPositionX = 0;
	mapPositionY = 0;

	getblock = (x, y) => {
		if (x % 10 === 0)
			return this.images.diamond_ore;
		else if (y % 10 === 0)
			return this.images.emerald_ore;
		else
			return this.images.stone;
	}

	draw = () => {
		if (this.canvas) {
			this.canvasContext.clearRect(0, 0, this.canvas.width, this.canvas.height)
			for (let x = Math.round(- this.canvas.width / 2 / 16 / this.mapScale); x <= Math.round(this.canvas.width / 2 / 16 / this.mapScale); x++)for (let y = Math.round(- this.canvas.height / 2 / 16 / this.mapScale); y <= Math.round(this.canvas.height / 2 / 16 / this.mapScale); y++) {
				this.canvasContext.drawImage(
					this.getblock(x, y),
					0, 0, 16, 16,
					this.canvas.width / 2 + (x - 0.5 + this.mapPositionX) * 16 * this.mapScale,
					this.canvas.height / 2 + (y - 0.5 + this.mapPositionY) * 16 * this.mapScale,
					16 * this.mapScale,
					16 * this.mapScale);
			}
		}
	}

	zoomIn = () => {
		this.mapScale *= 1.2;
	}

	zoomOut = () => {
		this.mapScale /= 1.2
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
				<button onClick={() => this.mapPositionX -= 1}>{"<"}</button>
				<button onClick={() => this.mapPositionX += 1}>{">"}</button>
				<button onClick={() => this.mapPositionY -= 1}>^</button>
				<button onClick={() => this.mapPositionY += 1}>v</button>
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