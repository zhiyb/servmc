import React, { Component } from 'react';

import diamond_ore from './maps/diamond_ore.png'
import emerald_ore from './maps/emerald_ore.png'
import stone from './maps/stone.png'

import './maps.css';

class maps extends Component {
	state = {
		mapSize: [1000, 1000],
		mapMouseMove: [0, 0],
		mapWheelScale: 1,
	};

	canvas = null;
	canvasContext = null;
	images = {};

	mapScale = 1;
	mapPosition = [0, 0];

	mapMouseDownPos = null;
	mapWheelTimer = null;

	getBlock = (x, y) => {
		if (x % 10 === 0)
			return this.images.diamond_ore;
		else if (y % 10 === 0)
			return this.images.emerald_ore;
		else
			return this.images.stone;
	}

	getChunk = (x, y, s) => {

	}

	draw = () => {
		if (this.canvas) {
			//处理鼠标移动
			this.mapPosition[0] += this.state.mapMouseMove[0] / 16 / this.mapScale;
			this.mapPosition[1] += this.state.mapMouseMove[1] / 16 / this.mapScale;
			this.mapMouseDownPos = null;
			this.setState({ mapMouseMove: [0, 0] });
			//处理滚轮缩放
			this.mapScale *= this.state.mapWheelScale;
			this.setState({ mapWheelScale: 1 });
			this.mapWheelTimer = null;

			this.canvasContext.clearRect(0, 0, this.canvas.width, this.canvas.height)
			if (this.mapScale > 0.5) {
				for (let x = Math.round(- this.canvas.width / 2 / 16 / this.mapScale - this.mapPosition[0]); x <= Math.round(this.canvas.width / 2 / 16 / this.mapScale - this.mapPosition[0]); x++)for (let y = Math.round(- this.canvas.height / 2 / 16 / this.mapScale - this.mapPosition[1]); y <= Math.round(this.canvas.height / 2 / 16 / this.mapScale - this.mapPosition[1]); y++) {
					this.canvasContext.drawImage(
						this.getBlock(x, y),
						0, 0, 16, 16,
						this.canvas.width / 2 + (x - 0.5 + this.mapPosition[0]) * 16 * this.mapScale,
						this.canvas.height / 2 + (y - 0.5 + this.mapPosition[1]) * 16 * this.mapScale,
						16 * this.mapScale,
						16 * this.mapScale);
				}
			}
		}
	}

	componentDidMount() {
		(this.images.diamond_ore = new Image()).src = diamond_ore;
		(this.images.emerald_ore = new Image()).src = emerald_ore;
		(this.images.stone = new Image()).src = stone;
	}

	handleMouseDown = event => {
		event.preventDefault();
		this.mapMouseDownPos = [event.clientX, event.clientY];
	}
	handleMouseMove = event => {
		if (this.mapMouseDownPos) {
			event.preventDefault();
			this.setState({
				mapMouseMove: [
					event.clientX - this.mapMouseDownPos[0],
					event.clientY - this.mapMouseDownPos[1]
				]
			});
		}
	}
	handleMouseUp = () => {
		if (this.mapMouseDownPos) {
			this.draw();
		}
	}

	handleWheel = event => {
		if (this.mapWheelTimer) clearTimeout(this.mapWheelTimer);	//如果已经有定时器清除掉
		this.mapWheelTimer = setTimeout(this.draw, 200);	//滚动停止后0.2s才重新渲染

		let mapScaleNew = this.mapScale * this.state.mapWheelScale * (1 + event.deltaY / 500);
		if (mapScaleNew < 100 && mapScaleNew > 0.125) {
			this.setState({ mapWheelScale: this.state.mapWheelScale * (1 + event.deltaY / 500) });
		}
	}

	render() {
		return (
			<div>
				<button onClick={this.draw}>draw</button>
				<br />
				<div className="map" style={{ height: this.state.mapSize[0], width: this.state.mapSize[1] }}
					onMouseDown={this.handleMouseDown}
					onMouseMove={this.handleMouseMove}
					onMouseUp={this.handleMouseUp}
					onMouseOut={this.handleMouseUp}
					onWheel={this.handleWheel}
				>
					<canvas
						className="canvas"
						ref={canvas => {
							this.canvas = canvas;
							if (canvas) {
								this.canvasContext = canvas.getContext("2d");
								this.canvasContext.imageSmoothingEnabled = false;
							}
						}}
						height={this.state.mapSize[0] * 3}
						width={this.state.mapSize[1] * 3}
						style={{
							left: this.state.mapMouseMove[0] - this.state.mapSize[0] + (this.state.mapSize[0] * 3 - this.state.mapSize[0] * 3 * this.state.mapWheelScale) / 2,
							top: this.state.mapMouseMove[1] - this.state.mapSize[1] + (this.state.mapSize[1] * 3 - this.state.mapSize[1] * 3 * this.state.mapWheelScale) / 2,
							width: this.state.mapSize[0] * 3 * this.state.mapWheelScale,
							height: this.state.mapSize[1] * 3 * this.state.mapWheelScale,
						}}
					></canvas>
				</div>
			</div >
		);
	}
}

export default maps;