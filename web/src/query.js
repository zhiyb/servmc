import request from './request';

let query = (data) => {
	let hostname;
	let port;
	if (process.env.NODE_ENV === 'development') {	//开发者模式
		hostname = "localhost";
		port = "25580";
	}
	else {
		hostname = window.location.hostname;
		port = window.location.port;
	}

	return request({
		hostname: hostname,
		port: port,
		path: '/q',
		method: 'POST',
		data: data,
	});
};

export default query;
