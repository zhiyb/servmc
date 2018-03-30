import React, { Component } from 'react';

import diamond_ore from './maps/diamond_ore.png'
import emerald_ore from './maps/emerald_ore.png'
import stone from './maps/stone.png'

import './maps.css';

class maps extends Component {
	state = {
		mapSize: [1000, 1000],
		mapMouseMove: [0, 0],
	};

	canvas = null;
	canvasContext = null;
	images = {};

	canvasMouseDownPos = null;

	mapScale = 1;
	mapPosition = [0, 0];

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
			this.setState({ mapMouseMove: [0, 0] });
			for (let x = Math.round(- this.canvas.width / 2 / 16 / this.mapScale - this.mapPosition[0]); x <= Math.round(this.canvas.width / 2 / 16 / this.mapScale - this.mapPosition[0]); x++)for (let y = Math.round(- this.canvas.height / 2 / 16 / this.mapScale - this.mapPosition[1]); y <= Math.round(this.canvas.height / 2 / 16 / this.mapScale - this.mapPosition[1]); y++) {
				this.canvasContext.drawImage(
					this.getblock(x, y),
					0, 0, 16, 16,
					this.canvas.width / 2 + (x - 0.5 + this.mapPosition[0]) * 16 * this.mapScale,
					this.canvas.height / 2 + (y - 0.5 + this.mapPosition[1]) * 16 * this.mapScale,
					16 * this.mapScale,
					16 * this.mapScale);
			}
		}
	}

	zoomIn = () => {
		this.mapScale *= 1.2;
		this.draw();
	}

	zoomOut = () => {
		this.mapScale /= 1.2
		this.draw();
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
				<div className="map" style={{ height: this.state.mapSize[0], width: this.state.mapSize[1] }}>
					<canvas
						className="canvas"
						ref={canvas => { this.canvas = canvas; this.canvasContext = canvas ? canvas.getContext("2d") : null; }}
						onMouseDown={event => {
							event.preventDefault();
							this.canvasMouseDownPos = [event.clientX, event.clientY];
						}}
						onMouseMove={event => {
							if (this.canvasMouseDownPos) {
								event.preventDefault();
								this.setState({
									mapMouseMove: [
										event.clientX - this.canvasMouseDownPos[0],
										event.clientY - this.canvasMouseDownPos[1]
									]
								});
							}
						}}
						onMouseUp={event => {
							if (this.canvasMouseDownPos) {
								event.preventDefault();
								this.mapPosition[0] += this.state.mapMouseMove[0] / 16 / this.mapScale;
								this.mapPosition[1] += this.state.mapMouseMove[1] / 16 / this.mapScale; this.canvasMouseDownPos = null; this.draw();
							}
						}}
						height={this.state.mapSize[0] * 3}
						width={this.state.mapSize[1] * 3}
						style={{ left: -this.state.mapSize[0] + this.state.mapMouseMove[0], top: -this.state.mapSize[1] + this.state.mapMouseMove[1] }}
					></canvas>
				</div>
			</div >
		);
	}
}

export default maps;